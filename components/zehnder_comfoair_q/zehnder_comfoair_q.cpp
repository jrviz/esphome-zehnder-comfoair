#include "zehnder_comfoair_q.h"

namespace zehnder_comfoair_q
{
    // Descriptions of the active-error bits (bit index == error ID), worded as in section 9.5
    // "ComfoAirQ alarm codes" (Table 2) of Zehnder's ComfoConnect PRO Modbus installation manual,
    // revision 137, as "<description> (<code>)". Entries marked "NOT in Zehnder manual" are not
    // covered by that table; they are kept from the aiocomfoconnect protocol notes, unverified.
    struct ErrorDescription
    {
        uint16_t id;
        const char *text;
    };
    static const ErrorDescription ERROR_DESCRIPTIONS[] = {
        {21, "Two or more temperature sensors are out of boundaries (HRU_T_FIRE_ERR)"},
        {22, "Temperature too high for HRU (T_HRU_ERR)"},
        {23, "Temperature sensor T11 value exceeded limit too often (T_11_ERR)"},
        {24, "Temperature sensor T11 value exceeds limit (T_11_LIMIT_ERR)"},
        {25, "Temperature sensor T12 value exceeded limit too often (T_12_ERR)"},
        {26, "Temperature sensor T12 value exceeds limit (T_12_LIMIT_ERR)"},
        {27, "Temperature sensor T20 value exceeded limit too often (T_20_ERR)"},
        {28, "Temperature sensor T20 value exceeds limit (T_20_LIMIT_ERR)"},
        {29, "Temperature sensor T21 value exceeded limit too often (T_21_ERR)"},
        {30, "Temperature sensor T21 value exceeds limit (T_21_LIMIT_ERR)"},
        {31, "Temperature sensor T22 value exceeded limit too often (T_22_ERR)"},
        {32, "Temperature sensor T22 value exceeds limit (T_22_LIMIT_ERR)"},
        {33, "HRU has not been initialized (HRU_INIT_ERR)"},
        {34, "The front door is open (HRU_FRONT_OPEN_ERR)"},
        {35, "Preheater is present, but its location (left/right) does not match the orientation of the HRU (H_21_release_ERR)"},
        {37, "Preheater fails to deliver requested power (H_21_P_ERR)"},
        {38, "Preheater fails to deliver requested power in required ratio (H_21_P_ratio_ERR)"},
        {39, "Humidity sensor ϕ11 value exceeded limit too often (ϕ_11_ERR)"},
        {41, "Humidity sensor ϕ12 value exceeded limit too often (ϕ_12_ERR)"},
        {43, "Humidity sensor ϕ20 value exceeded limit too often (ϕ_20_ERR)"},
        {45, "Humidity sensor ϕ21 value exceeded limit too often (ϕ_21_ERR)"},
        {47, "Humidity sensor ϕ22 value exceeded limit too often (ϕ_22_ERR)"},
        {49, "Pressure sensor P12 value exceeded limit too often (P_12_ERR)"},
        {50, "Pressure sensor P22 value exceeded limit too often (P_22_ERR)"},
        {51, "Speed of F12 fan exceeded limit too often (F_12_s_ERR)"},
        {52, "Speed of F22 fan exceeded limit too often (F_22_s_ERR)"},
        {53, "Static pressure of sensor P12 exceeded limit too often (Ptot_12_s_ERR)"},
        {54, "Static pressure of sensor P22 exceeded limit too often (Ptot_22_s_ERR)"},
        {55, "Failed to reach required F12 fan speed too often (F_12_s_set_ERR)"},
        {56, "Failed to reach required F22 fan speed too often (F_22_s_set_ERR)"},
        {57, "Failed to reach required mass flow for F12 fan too often (Qm_12_set_ERR)"},
        {58, "Failed to reach required mass flow for F22 fan too often (Qm_22_set_ERR)"},
        {59, "Failed to reach required temperature too often for outdoor air after preheater (T_21_set_ERR)"},
        {60, "Failed to reach required temperature too often for supply air (T_22_set_ERR)"},
        {61, "Supply air temperature (sensor T22) is too low too often (T_22_frost_ERR)"},
        {62, "Unbalance occurred too often beyond tolerance levels in past period (Unbalance_ERR)"},
        {63, "Postheater was present, but is no longer detected (POSTHEAT_CONNECT ERROR)"}, // NOT in Zehnder manual 9.5 - unverified, kept from aiocomfoconnect
        {64, "Temperature sensor value for supply air ComfoCool exceeded limit too often (CCOOL_TEMP ERROR)"}, // NOT in Zehnder manual 9.5 - unverified, kept from aiocomfoconnect
        {65, "Room temperature sensor was present, but is no longer detected (T_ROOM_PRES ERROR)"}, // NOT in Zehnder manual 9.5 - unverified, kept from aiocomfoconnect
        {66, "RF Communication hardware was present, but is no longer detected (Present_RF_ERR)"},
        {67, "Option board was present, but is no longer detected (Present_IO_ERR)"},
        {68, "Preheater was present, but is no longer detected (Present_H_21_ERR)"},
        {69, "Post heater was present, but is no longer detected (Present_H_23_ERR)"},
        {70, "Analog input 1 was present, but is no longer detected (ANALOG_1_PRES ERROR)"}, // NOT in Zehnder manual 9.5 - unverified, kept from aiocomfoconnect
        {71, "Analog input 2 was present, but is no longer detected (ANALOG_2_PRES ERROR)"}, // NOT in Zehnder manual 9.5 - unverified, kept from aiocomfoconnect
        {72, "Analog input 3 was present, but is no longer detected (ANALOG_3_PRES ERROR)"}, // NOT in Zehnder manual 9.5 - unverified, kept from aiocomfoconnect
        {73, "Analog input 4 was present, but is no longer detected (ANALOG_4_PRES ERROR)"}, // NOT in Zehnder manual 9.5 - unverified, kept from aiocomfoconnect
        {74, "Cooker hood was present, but is no longer detected (Present_Hood_ERR)"},
        {75, "Comfo Cool was present, but is no longer detected (Present_CCOOL_ERR)"},
        {76, "ComfoFond was present, but is no longer detected (Present_G_ERR)"},
        {77, "Filters must be replaced now (Filter_alarm_flag)"},
        {78, "The external filter input is high (Filter_ext_ERR)"},
        {79, "Filters must be ordered now, because the remaining filter lifetime is limited (Filter_warning_flag)"},
        {80, "Standby is active (Standby_ERR)"},
        {81, "Preheater does not communicate reliably (H_21_COMM_ERR)"},
        {82, "ComfoHood temperature error (HOOD_TEMP ERROR)"}, // NOT in Zehnder manual 9.5 - unverified, kept from aiocomfoconnect
        {83, "Postheater temperature error (POSTHEAT_TEMP ERROR)"}, // NOT in Zehnder manual 9.5 - unverified, kept from aiocomfoconnect
        {84, "Outdoor temperature of ComfoFond error (GROUND_HEAT_TEMP ERROR)"}, // NOT in Zehnder manual 9.5 - unverified, kept from aiocomfoconnect
        {85, "Analog input 1 error (ANALOG_1_IN ERROR)"}, // NOT in Zehnder manual 9.5 - unverified, kept from aiocomfoconnect
        {86, "Analog input 2 error (ANALOG_2_IN ERROR)"}, // NOT in Zehnder manual 9.5 - unverified, kept from aiocomfoconnect
        {87, "Analog input 3 error (ANALOG_3_IN ERROR)"}, // NOT in Zehnder manual 9.5 - unverified, kept from aiocomfoconnect
        {88, "Analog input 4 error (ANALOG_4_IN ERROR)"}, // NOT in Zehnder manual 9.5 - unverified, kept from aiocomfoconnect
        {89, "Bypass is manual used. (T_22_MANUAL_ERR)"},
        {90, "ComfoCool is overheated (CC_OVERHEAT_ERR)"},
        {91, "ComfoCool compressor error (CC_COMP_ERR)"},
        {92, "ComfoCool room temperature out of bounds (CC_T_10_ERR)"},
        {93, "ComfoCool compressor temperature out of bounds (CC_T_13_ERR)"},
        {94, "ComfoCool supply temperature out of bounds (CC_T_23_ERR)"},
        {95, "Hood temperature is too high (T_hood_ERR)"},
        {96, "Hood is activated (IO_hood_duty_ERR)"},
        {97, "Status flag (QM_Constraint_min_ERR)"},
        {98, "Flow too low for preheater (H_21_qm_min_ERR)"},
        {99, "Configuration error (config_ERR)"},
        {100, "Warning that error analysis is in progress (analysis_busy_warning)"},
        {101, "Error on the ComfoNet bus (ComfoNet_ERR)"},
        {102, "The number of CO2 sensors on a controller has decreased – one or more sensors are no longer detected (CO2_SENS_COUNT_ERR)"},
        {103, "More than 8 sensors are detected in a zone (CO2_SENS_TOO_MANY_ERR)"},
        {104, "General CO2 sensor error (CO2_SENS_GENERAL_ERR)"},
    };
    static const char *const NO_ACTIVE_ERRORS = "No active errors";
    static const char *const UNKNOWN_ERROR = "Unknown error";
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

        if (active_error_ids.empty())
            return NO_ACTIVE_ERRORS;

        std::string result;
        for (auto error_id : active_error_ids)
        {
            if (!result.empty())
                result += "; ";

            const char *description = UNKNOWN_ERROR;
            for (const auto &entry : ERROR_DESCRIPTIONS)
            {
                if (entry.id == error_id)
                {
                    description = entry.text;
                    break;
                }
            }
            result += to_string(error_id) + ": " + description;
        }
        return result;
    }
}
