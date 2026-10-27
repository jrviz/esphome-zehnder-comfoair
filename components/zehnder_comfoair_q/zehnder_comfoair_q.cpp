#include "zehnder_comfoair_q.h"

// The error descriptions language is a compile-time choice: __init__.py defines
// ZEHNDER_ERROR_DESCRIPTIONS_HEADER from the `error_language:` YAML option. All language files
// are seen (empty) via esphome.h; defining ZEHNDER_ERROR_DESCRIPTIONS_SELECTED only around this
// include switches on the data of the one chosen language, so only that language is compiled in.
#ifndef ZEHNDER_ERROR_DESCRIPTIONS_HEADER
#define ZEHNDER_ERROR_DESCRIPTIONS_HEADER "error_descriptions_en.h"
#endif
#define ZEHNDER_ERROR_DESCRIPTIONS_SELECTED
#include ZEHNDER_ERROR_DESCRIPTIONS_HEADER
#undef ZEHNDER_ERROR_DESCRIPTIONS_SELECTED

namespace zehnder_comfoair_q
{
    // Home Assistant rejects entity states longer than 255 characters.
    static const size_t MAX_STATE_LENGTH = 255;

    void ZehnderComfoAirQ::setup()
    {
        // do a first request of all PDOs some time after starting (helpful for long intervals)
        this->set_timeout(1000 * 10, [this]()
                          { this->update(); });

        if (this->error_poll_interval_ > 0 && this->error_poll_interval_ != SCHEDULER_DONT_RUN)
        {
            // stagger the first errors poll away from the initial PDO burst above
            this->set_timeout(1000 * 15, [this]()
                              { this->request_active_errors(); });
            this->set_interval("active_errors_poll", this->error_poll_interval_, [this]()
                               { this->request_active_errors(); });
        }
    }

    void ZehnderComfoAirQ::update()
    {
        if (request_ids_.size() > 0)
            request_all_pdos();
    }

    void ZehnderComfoAirQ::request_all_pdos()
    {
        ESP_LOGD(TAG, "Requesting all registered PDOs (count: %d)", request_ids_.size());

        if (request_next_pdo_pos_ == 0)
            request_next_pdo_();
        else
            ESP_LOGW(TAG, "Skipping PDO request cycle as last cycle is still in progress.");
    }

    void ZehnderComfoAirQ::request_next_pdo_()
    {
        if (request_next_pdo_pos_ >= request_ids_.size())
            return;

        this->request_pdo(request_ids_[request_next_pdo_pos_]);
        request_next_pdo_pos_++;
        if (request_next_pdo_pos_ < request_ids_.size())
        {
            this->set_timeout(this->request_delay_, [this]()
                              { this->request_next_pdo_(); });
        }
        else
        {
            request_next_pdo_pos_ = 0;
        }
    }

    void ZehnderComfoAirQ::request_pdo(uint16_t pdo_id)
    {
        // ZehnderComfoAirQ does not care for the data length to be set corectly on transmission requests, so there is no need to send any data
        this->send_can_message_((pdo_id << 14) + 0x40 + this->local_node_id_, true);
    }

    void ZehnderComfoAirQ::set_level(uint8_t level)
    {
        send_command_set_timer(true, 0x01, 0x01, level);
    }

    void ZehnderComfoAirQ::set_level_float(float state)
    {
        uint8_t level;
        if (state >= 1)
            level = 3;
        else if (state >= 0.5)
            level = 2;
        else if (state > 0)
            level = 1;
        else
            level = 0;
        ESP_LOGD(TAG, "send_cmd_level_float: state: %f, level: %d", state, level);

        set_level(level);
    }

    void ZehnderComfoAirQ::send_command_set_timer(bool enable, uint8_t subunit_id, uint8_t property_id, uint8_t property_value, uint32_t duration_secs)
    {
        std::vector<uint8_t> command = {(uint8_t)(enable ? 0x84 : 0x85), 0x15 /* SCHEDULE */, subunit_id, property_id};
        if (enable)
            command.insert(command.end(), {0x00, 0x00, 0x00, 0x00, (uint8_t)(duration_secs), (uint8_t)(duration_secs >> 8),
                                           (uint8_t)(duration_secs >> 16), (uint8_t)(duration_secs >> 24), property_value});
        send_command(command);
    }

    void ZehnderComfoAirQ::send_command_set_property(uint8_t unit_id, uint8_t subunit_id, uint8_t property_id, uint8_t property_value)
    {
        send_command({0x03, unit_id, subunit_id, property_id, property_value});
    }

    void ZehnderComfoAirQ::send_command(const std::vector<uint8_t> &command)
    {
        auto is_multi_message_command = command.size() > 8;
        auto can_id = get_command_can_id_(is_multi_message_command, get_command_next_sequence_number_());

        if (is_multi_message_command)
        {
            std::vector<uint8_t> message_buffer;
            int message_counter = 0;
            for (size_t offset = 0; offset < command.size(); offset += 7)
            {
                auto length = std::min(size_t(7), command.size() - offset);
                auto command_pos = command.begin() + offset;
                auto is_last_message = offset + length == command.size();
                message_buffer.clear();
                message_buffer.push_back(message_counter | (is_last_message ? 0x80 : 0));
                message_buffer.insert(message_buffer.end(), command_pos, command_pos + length);
                send_can_message_(can_id, false, message_buffer);

                message_counter++;
            }
        }
        else
        {
            send_can_message_(can_id, false, command);
        }
    }

    uint32_t ZehnderComfoAirQ::get_command_can_id_(bool is_multi_message_command, uint8_t sequence_number)
    {
        return 0x1f << 24 |
               sequence_number << 17 |
               1 << 16 | // is_request
               (is_multi_message_command ? 1 : 0) << 14 |
               0x01 << 6 | // destination node: the ComfoAir Q unit
               this->local_node_id_;
    }

    uint8_t ZehnderComfoAirQ::get_command_next_sequence_number_()
    {
        this->command_sequence_number_ = (this->command_sequence_number_ + 1) & 0x3;
        return this->command_sequence_number_;
    }

    void ZehnderComfoAirQ::send_can_message_(uint32_t can_id, bool remote_transmission_request, const std::vector<uint8_t> &data)
    {
        ESP_LOGD(TAG, "Send can message: id: 0x%08lx (pdo_id: %ld), rtr: %d, size: %d, content: %s", can_id, can_id >> 14,
                 remote_transmission_request, data.size(), format_hex_pretty(data).c_str());

        if (parent_ == nullptr)
        {
            ESP_LOGE(TAG, "Parent not set, exiting send_can_message_.");
            return;
        }

        // We could have also called send_data directly, but using CanbusSendAction seem to be a bit more future-proof
        // id(parent_).send_data(can_id, true, true, {});

        CanbusSendAction<> canbus_send_action;
        canbus_send_action.set_parent(parent_);
        canbus_send_action.set_can_id(can_id);
        // use_extended_id has already been set globally on parent
        if (remote_transmission_request)
            canbus_send_action.set_remote_transmission_request(true);
        canbus_send_action.set_data_static(data.data(), data.size());
        canbus_send_action.play();
    }

    // --- RMI request/response framework ---

    void ZehnderComfoAirQ::send_rmi_request(const std::vector<uint8_t> &command, RmiResponseCallback callback, uint32_t timeout_ms)
    {
        this->rmi_request_queue_.push_back({command, callback, timeout_ms});
        if (!this->rmi_request_in_flight_)
            this->rmi_send_next_request_();
    }

    void ZehnderComfoAirQ::rmi_send_next_request_()
    {
        if (this->rmi_request_queue_.empty())
            return;

        const auto &request = this->rmi_request_queue_.front();
        this->rmi_request_in_flight_ = true;
        this->rmi_response_buffer_.clear();
        this->rmi_next_block_ = 0;
        this->rmi_current_callback_ = request.callback;

        this->send_command(request.command);

        this->set_timeout("rmi_request_timeout", request.timeout_ms, [this]()
                          {
                              ESP_LOGW(TAG, "Timed out waiting for RMI response");
                              this->rmi_complete_request_(false);
                          });
    }

    void ZehnderComfoAirQ::receive_rmi_frame(uint32_t can_id, const std::vector<uint8_t> &data)
    {
        if (!this->rmi_request_in_flight_)
            return;

        uint8_t src_node_id = can_id & 0x3f;
        uint8_t dst_node_id = (can_id >> 6) & 0x3f;
        bool response_error_occurred = (can_id >> 15) & 0x1;
        bool is_multi_message = (can_id >> 14) & 0x1;

        // we only care about responses from the ventilation unit addressed to us
        if (src_node_id != 0x01 || dst_node_id != this->local_node_id_)
            return;

        if (response_error_occurred)
        {
            ESP_LOGW(TAG, "RMI request returned an error response");
            this->rmi_complete_request_(false);
            return;
        }

        if (!is_multi_message)
        {
            this->rmi_response_buffer_ = data;
            this->rmi_complete_request_(true);
            return;
        }

        // multi-frame message: each frame is prefixed with a block index (0x80 flag = last block)
        if (data.empty())
            return;

        uint8_t block_index = data[0] & 0x7f;
        bool is_last_block = data[0] & 0x80;

        if (block_index != this->rmi_next_block_)
        {
            ESP_LOGW(TAG, "Unexpected RMI response block %d (expected %d)", block_index, this->rmi_next_block_);
            this->rmi_complete_request_(false);
            return;
        }
        this->rmi_next_block_++;

        this->rmi_response_buffer_.insert(this->rmi_response_buffer_.end(), data.begin() + 1, data.end());

        if (is_last_block)
            this->rmi_complete_request_(true);
    }

    void ZehnderComfoAirQ::rmi_complete_request_(bool success)
    {
        this->cancel_timeout("rmi_request_timeout");

        auto callback = this->rmi_current_callback_;
        auto response = this->rmi_response_buffer_;

        this->rmi_current_callback_ = nullptr;
        this->rmi_response_buffer_.clear();
        this->rmi_request_in_flight_ = false;
        if (!this->rmi_request_queue_.empty())
            this->rmi_request_queue_.pop_front();

        if (callback)
            callback(success, response);

        // send the next queued request, if any (and if the callback above didn't already start one)
        if (!this->rmi_request_in_flight_)
            this->rmi_send_next_request_();
    }

    // --- Active errors (GETACTIVEERRORS), built on top of the RMI framework ---

    void ZehnderComfoAirQ::request_active_errors()
    {
        ESP_LOGD(TAG, "Requesting active errors (GETACTIVEERRORS)");

        this->send_rmi_request({RMI_CMD_GET_ACTIVE_ERRORS, RMI_UNIT_ERROR, 0x01 /* SubUnit */}, [this](bool success, const std::vector<uint8_t> &response)
                               {
                                   if (!success)
                                   {
                                       ESP_LOGW(TAG, "Failed to retrieve active errors");
                                       return;
                                   }

                                   std::vector<uint16_t> active_error_ids;
                                   std::string result = format_active_errors_(response, active_error_ids);
                                   ESP_LOGD(TAG, "Active errors: %s", result.c_str());

                                   if (result.size() > MAX_STATE_LENGTH)
                                   {
                                       // cut at a character boundary, not in the middle of a UTF-8 sequence
                                       size_t cut = MAX_STATE_LENGTH - 3;
                                       while (cut > 0 && (static_cast<uint8_t>(result[cut]) & 0xC0) == 0x80)
                                           cut--;
                                       result = result.substr(0, cut) + "...";
                                   }

                                   if (this->active_errors_text_sensor_ != nullptr)
                                       this->active_errors_text_sensor_->publish_state(result);
                                   if (this->has_active_errors_binary_sensor_ != nullptr)
                                       this->has_active_errors_binary_sensor_->publish_state(!active_error_ids.empty());
                               });
    }

    void ZehnderComfoAirQ::reset_all_errors()
    {
        ESP_LOGD(TAG, "Resetting all errors (RESETALLERRORS)");

        this->send_rmi_request({RMI_CMD_RESET_ALL_ERRORS, RMI_UNIT_ERROR, 0x01 /* SubUnit */}, [this](bool success, const std::vector<uint8_t> &response)
                               {
                                   if (!success)
                                   {
                                       ESP_LOGW(TAG, "Failed to reset errors");
                                       return;
                                   }

                                   // errors are only polled periodically, so read them back straight away
                                   // rather than waiting for the next error_poll_interval
                                   this->request_active_errors();
                               });
    }

    std::string ZehnderComfoAirQ::format_active_errors_(const std::vector<uint8_t> &data, std::vector<uint16_t> &active_error_ids)
    {
        for (size_t byte_index = 0; byte_index < data.size(); byte_index++)
        {
            for (uint8_t bit = 0; bit < 8; bit++)
            {
                if (data[byte_index] & (1 << bit))
                    active_error_ids.push_back(byte_index * 8 + bit);
            }
        }

        const auto &table = error_descriptions::TABLE;

        if (active_error_ids.empty())
            return table.no_active_errors;

        std::string result;
        for (auto error_id : active_error_ids)
        {
            if (!result.empty())
                result += "; ";

            const char *description = table.unknown_error;
            for (size_t i = 0; i < table.count; i++)
            {
                if (table.entries[i].id == error_id)
                {
                    description = table.entries[i].text;
                    break;
                }
            }
            result += to_string(error_id) + ": " + description;
        }
        return result;
    }
}
