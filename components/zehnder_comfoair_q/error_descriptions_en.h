// NOTE: no #pragma once / include guard here on purpose - see ZEHNDER_ERROR_DESCRIPTIONS_SELECTED below.
//
// Human-readable descriptions for each active-error bit (bit index == error ID), firmware 1.4.0+
// numbering. See PROTOCOL-RMI.md "ERROR Unit commands" in michaelarnauts/aiocomfoconnect.
//
// Source of the wording: the ERRORS table (firmware 1.4.0+ numbering) in
// aiocomfoconnect/const.py of michaelarnauts/aiocomfoconnect, used verbatim. It is the master text.
// Its descriptions say which part failed (for example "extract air fan", "supply air temperature
// sensor"), which is why it is preferred.
//
// These descriptions are copied from aiocomfoconnect, which is MIT licensed:
// Copyright (c) 2022 Michaël Arnauts, https://github.com/michaelarnauts/aiocomfoconnect (see its LICENSE)
//
// Cross-checked against section 9.5 "ComfoAirQ alarm codes" (Table 2) of Zehnder's "ComfoConnect PRO
// Modbus TCP/RTU installation" manual (revision 137): every ID the manual lists is in this table, but
// the manual words some of them differently (it names sensors by designation, T11, P12, F12, ... and
// calls ID 80 "Standby is active" where this table says "Service mode is active") and it has no entry
// for IDs 63-65, 70-73 and 82-88.
// The no_active_errors / unknown_error strings are ours.
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
            {21, "DANGER! OVERHEATING! Two or more sensors are detecting an incorrect temperature. Ventilation has stopped."},
            {22, "Temperature too high for ComfoAir Q (TEMP_HRU ERROR)"},
            {23, "The extract air temperature sensor has a malfunction (SENSOR_ETA ERROR)"},
            {24, "The extract air temperature sensor is detecting an incorrect temperature (TEMP_SENSOR_ETA ERROR)"},
            {25, "The exhaust air temperature sensor has a malfunction (SENSOR_EHA ERROR)"},
            {26, "The exhaust air temperature sensor is detecting an incorrect temperature (TEMP_SENSOR_EHA ERROR)"},
            {27, "The outdoor air temperature sensor has a malfunction (SENSOR_ODA ERROR)"},
            {28, "The outdoor air temperature sensor is detecting an incorrect temperature (TEMP_SENSOR_ODA ERROR)"},
            {29, "The pre-conditioned outdoor air temperature sensor has a malfunction"},
            {30, "The pre-conditioned outdoor air temperature sensor is detecting an incorrect temperature (TEMP_SENSOR_P-ODA ERROR)"},
            {31, "The supply air temperature sensor has a malfunction (SENSOR_SUP ERROR)"},
            {32, "The supply air temperature sensor is detecting an incorrect temperature (TEMP_SENSOR_SUP ERROR)"},
            {33, "The Ventilation Unit has not been commissioned (INIT ERROR)"},
            {34, "The front door is open"},
            {35, "The Pre-heater is present, but not in the correct position (right/left). (PREHEAT_LOCATION ERROR)"},
            {37, "The pre-heater has a malfunction (PREHEAT ERROR)"},
            {38, "The pre-heater has a malfunction (PREHEAT ERROR)"},
            {39, "The extract air humidity sensor has a malfunction (SENSOR_ETA ERROR)"},
            {41, "The exhaust air humidity sensor has a malfunction (SENSOR_EHA ERROR)"},
            {43, "The outdoor air humidity sensor has a malfunction (SENSOR_ODA ERROR)"},
            {45, "The outdoor air humidity sensor has a malfunction (SENSOR_P-ODA ERROR)"},
            {47, "The supply air humidity sensor has a malfunction (SENSOR_SUP ERROR)"},
            {49, "The exhaust air flow sensor has a malfunction (SENSOR_EHA ERROR)"},
            {50, "The supply air flow sensor has a malfunction (SENSOR_SUP ERROR)"},
            {51, "The extract air fan has a malfunction (FAN_EHA ERROR)"},
            {52, "The supply air fan has a malfunction (FAN_SUP ERROR)"},
            {53, "Exhaust air pressure too high. Check air outlets, ducts and filters for pollution and obstructions. Check valve settings (EXT_PRESSURE_EHA ERROR)"},
            {54, "Supply air pressure too high. Check air outlets, ducts and filters for pollution and obstructions. Check valve settings. (EXT_PRESSURE_SUP ERROR)"},
            {55, "The extract air fan has a malfunction (FAN_EHA ERROR)"},
            {56, "The supply air fan has a malfunction (FAN_SUP ERROR)"},
            {57, "The exhaust air flow is not reaching its set point (AIRFLOW_EHA ERROR)"},
            {58, "The supply air flow is not reaching its set point (AIRFLOW_SUP ERROR)"},
            {59, "Failed to reach required temperature too often for outdoor air after pre-heater (TEMPCONTROL_P-ODA ERROR)"},
            {60, "Failed to reach required temperature too often for supply air. The modulating by-pass may have a malfunction. (TEMPCONTROL_SUP ERROR)"},
            {61, "Supply air temperature is too low too often (TEMP_SUP_MIN ERROR)"},
            {62, "Unbalance occurred too often beyond tolerance levels in past period (UNBALANCE ERROR)"},
            {63, "Postheater was present, but is no longer detected (POSTHEAT_CONNECT ERROR)"},
            {64, "Temperature sensor value for supply air ComfoCool exceeded limit too often (CCOOL_TEMP ERROR)"},
            {65, "Room temperature sensor was present, but is no longer detected (T_ROOM_PRES ERROR)"},
            {66, "RF Communication hardware was present, but is no longer detected (RF_PRES ERROR)"},
            {67, "Option Box was present, but is no longer detected (OPTION_BOX CONNECT ERROR)"},
            {68, "Pre-heater was present, but is no longer detected (PREHEAT_PRES ERROR)"},
            {69, "Postheater was present, but is no longer detected (POSTHEAT_CONNECT ERROR)"},
            {70, "Analog input 1 was present, but is no longer detected (ANALOG_1_PRES ERROR)"},
            {71, "Analog input 2 was present, but is no longer detected (ANALOG_2_PRES ERROR)"},
            {72, "Analog input 3 was present, but is no longer detected (ANALOG_3_PRES ERROR)"},
            {73, "Analog input 4 was present, but is no longer detected (ANALOG_4_PRES ERROR)"},
            {74, "ComfoHood was present, but is no longer detected (HOOD_CONNECT ERROR)"},
            {75, "ComfoCool was present, but is no longer detected (CCOOL_CONNECT ERROR)"},
            {76, "ComfoFond was present, but is no longer detected (GROUND_HEAT_CONNECT ERROR)"},
            {77, "The filters of the Ventilation Unit must be replaced now"},
            {78, "It is necessary to replace or clean the external filter"},
            {79, "Order new filters now, because the remaining filter life time is limited"},
            {80, "Service mode is active (SERVICE MODE)"},
            {81, "Preheater has no communication with the ComfoAir unit (PREHEAT ERROR , 1081)"},
            {82, "ComfoHood temperature error (HOOD_TEMP ERROR)"},
            {83, "Postheater temperature error (POSTHEAT_TEMP ERROR)"},
            {84, "Outdoor temperature of ComfoFond error (GROUND_HEAT_TEMP ERROR)"},
            {85, "Analog input 1 error (ANALOG_1_IN ERROR)"},
            {86, "Analog input 2 error (ANALOG_2_IN ERROR)"},
            {87, "Analog input 3 error (ANALOG_3_IN ERROR)"},
            {88, "Analog input 4 error (ANALOG_4_IN ERROR)"},
            {89, "Bypass is in manual mode"},
            {90, "ComfoCool is overheating"},
            {91, "ComfoCool compressor error (CCOOL_COMPRESSOR ERROR)"},
            {92, "ComfoCool room temperature sensor error (CCOOL_TEMP ERROR)"},
            {93, "ComfoCool condensor temperature sensor error (CCOOL_TEMP ERROR)"},
            {94, "ComfoCool supply air temperature sensor error (CCOOL_TEMP ERROR)"},
            {95, "ComfoHood temperature is too high (HOOD_TEMP ERROR)"},
            {96, "ComfoHood is activated"},
            {97, "QM_Constraint_min_ERR"},
            {98, "H_21_qm_min_ERR"},
            {99, "Configuration error"},
            {100, "Error analysis is in progress…"},
            {101, "ComfoNet Error"},
            {102, "The number of CO2 sensors has decreased – one or more sensors are no longer detected"},
            {103, "More than 8 sensors detected in a zone"},
            {104, "CO₂ Sensor C error"},
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
