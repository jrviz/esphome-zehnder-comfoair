// NOTE: no #pragma once / include guard here on purpose - see ZEHNDER_ERROR_DESCRIPTIONS_SELECTED below.
//
// Human-readable descriptions for each active-error bit (bit index == error ID), firmware 1.4.0+
// numbering. See PROTOCOL-RMI.md "ERROR Unit commands" in michaelarnauts/aiocomfoconnect.
//
// Source of truth for the wording: Zehnder "ComfoConnect PRO Modbus TCP/RTU installation" manual
// (Document Revision 137), section 9.5 "ComfoAirQ alarm codes", Table 2. Each text there is used
// verbatim as "<description> (<code>)", including its quirks (e.g. "Bypass is manual used."). The
// manual lists IDs 51 and 55 twice (identical rows). Entries marked "NOT in Zehnder manual" are
// not covered by that table; they are kept from the aiocomfoconnect protocol notes and unverified.
// The no_active_errors / unknown_error strings are ours, not from the manual.
//
// To add a new language:
//   1. Copy this file to error_descriptions_<code>.h (e.g. error_descriptions_nl.h) IN THIS SAME
//      FOLDER - ESPHome's local external_components copy step does not walk subdirectories, so
//      files have to live flat alongside zehnder_comfoair_q.cpp/.h, not in a subfolder. Keep the
//      same error IDs and the names ENTRIES / TABLE, keep the #ifdef/#endif wrapper exactly as it
//      is, and translate no_active_errors, unknown_error and every entry's text.
//   2. Set `error_language: <code>` on the zehnder_comfoair_q component in YAML.
// That's it - no C++ changes. The language is chosen at compile time: only the selected file is
// compiled into the firmware, the others are ignored (and unknown codes are rejected at config time).
//
// Why the #ifdef wrapper: ESPHome generates an esphome.h that #includes every .h in the component
// folder, so every language file is seen by every source file. The wrapper makes the file empty
// unless zehnder_comfoair_q.cpp defines ZEHNDER_ERROR_DESCRIPTIONS_SELECTED right before including
// the chosen language, so only that one language's data is emitted (once, in that one file).

#include "error_descriptions.h"

#ifdef ZEHNDER_ERROR_DESCRIPTIONS_SELECTED

namespace zehnder_comfoair_q
{
    namespace error_descriptions
    {
        static const Entry ENTRIES[] = {
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

        static const Table TABLE = {
            "No active errors",
            "Unknown error",
            ENTRIES,
            sizeof(ENTRIES) / sizeof(ENTRIES[0]),
        };
    }
}

#endif // ZEHNDER_ERROR_DESCRIPTIONS_SELECTED
