/**********************************************************************************
 *  _____ ___  _   _      ___  _____   _____ _    ___  ___ __  __ ___ _  _ _____ 
 * |_   _|   \| | | |    |   \| __\ \ / / __| |  / _ \| _ \  \/  | __| \| |_   _|
 *   | | | |) | |_| |    | |) | _| \ V /| _|| |_| (_) |  _/ |\/| | _|| .` | | |  
 *   |_| |___/ \___/     |___/|___| \_/ |___|____\___/|_| |_|  |_|___|_|\_| |_|  
 *
 **********************************************************************************/
/**
 * @file    debounce.h
 * @brief   Software debounce filter for digital inputs
 * @author  Théo DUPERRET
 * @see     <a href="https://github.com/tduperret">GitHub</a>
 * @see     <a href="https://gitlab.com/tduperret">GitLab</a>
 * @see     <a href="https://www.linkedin.com/in/duperret-theo">LinkedIn</a>
 * @date    24/06/2026 - Last Edit : 24/06/2026
 */

#ifndef DEBOUNCE_H
#define DEBOUNCE_H

    #include <stdint.h>
    #include <stdbool.h>

    /**
     * @brief Raw input read callback
     * 
     * This callback is used by the debounce module to sample the current state of the input signal
     * 
     * The returned value shall represent the raw, unfiltered signal level at the time of the call (true = high, false = low)
     * 
     * The implementation must be non-blocking and safe to call from the execution context in which the debounce module is updated
     * @param context User-defined context pointer provided during initialization. May be NULL if no context is required
     * @return Current raw signal level :
     *         - true  : logic high
     *         - false : logic low
     */
    typedef bool (*b_debounce_read_func_t)(void* context);

    /**
     * @brief Debounce module status codes
     */
    typedef enum __attribute__((__packed__))
    {
        e_DEBOUNCE_STATUS_ERROR_UNKNOWN               = 0,  /**< Unknown error */
        e_DEBOUNCE_STATUS_OK                          = 1,  /**< Operation completed successfully */
        e_DEBOUNCE_STATUS_ERROR_INVALID_INSTANCE      = 2,  /**< Invalid debounce instance */
        e_DEBOUNCE_STATUS_ERROR_INVALID_READ_FUNCTION = 3   /**< Invalid read callback function */
    } e_debounce_status_t;
    
    /**
     * @brief Debounced input logic level
     */
    typedef enum __attribute__((__packed__))
    {
        e_DEBOUNCE_LEVEL_UNKNOWN    = 0,    /**< Unknown logic level */
        e_DEBOUNCE_LEVEL_LOW        = 1,    /**< Logic low level */
        e_DEBOUNCE_LEVEL_HIGH       = 2     /**< Logic high level */
    } e_debounce_levels_t;

    /**
     * @brief Debounce event status
     */
    typedef enum __attribute__((__packed__))
    {
        e_DEBOUNCE_EVENT_UNKNOWN    = 0,    /**< Unknown event state */
        e_DEBOUNCE_EVENT_PENDING    = 1,    /**< Event is pending validation */
        e_DEBOUNCE_EVENT_NONE       = 2     /**< No event detected */
    } e_debounce_events_t;

    /**
     * @brief Debounce input instance
     *
     * This structure contains the runtime state and configuration of a debounced digital input
     */
    typedef struct
    {
        uint32_t ui32_id;              /**< Input identifier */
        void* context;                 /**< User context passed to the read function */
        b_debounce_read_func_t read;   /**< Input read callback */
        uint8_t ui8_cycles;            /**< Number of consecutive identical samples required to validate a state change. debounce_time = cycles / update_rate. */
        uint32_t ui32_history;         /**< Sample history shift register */
        uint32_t ui32_mask;            /**< History validation mask */
        bool b_level;                  /**< Current debounced level (true = high, false = low) */
        bool b_initialized;            /**< False until the first stable window has been acquired */
        bool b_rising_event;           /**< Validated low-to-high transition */
        bool b_falling_event;          /**< Validated high-to-low transition */
    } s_debounce_t;

    /**
     * @brief Initialize a debounce filter instance
     *
     * Configures a debounce filter and resets its internal state
     * 
     * The filter will begin collecting samples on subsequent calls to e_debounce_update()
     * @param ps_db Pointer to the debounce instance to initialize
     * @param ui32_id Application defined identifier associated with the instance
     * @param read Raw input read callback. Must not be NULL
     * @param context User context pointer passed to the read callback. May be NULL if not required
     * @param ui8_cycles Number of consecutive identical samples required to validate a state transition. Values are clamped to the range [1, 31]
     * @return Debounce operation status
     */
    e_debounce_status_t e_debounce_init(s_debounce_t* ps_db, uint32_t ui32_id, b_debounce_read_func_t read, void* context, uint8_t ui8_cycles);

    /**
     * @brief Reset a debounce filter instance
     *
     * Clears the filter history, pending events, and initialization state without modifying the filter configuration
     * @param ps_db Pointer to the debounce instance
     * @return Debounce operation status
     */
    e_debounce_status_t e_debounce_reset(s_debounce_t* ps_db);

    /**
     * @brief Sample the input and update the debounce filter
     *
     * Reads the current raw input state through the configured callback and updates the internal debounce filter state
     * @note This function should be called periodically at a fixed rate.
     * @param ps_db Pointer to the debounce instance
     * @return Debounce operation status
     */
    e_debounce_status_t e_debounce_update(s_debounce_t* ps_db);

    /**
     * @brief Get the current debounced input level
     * @param ps_DB Pointer to the debounce instance
     * @return Current debounced level
     */
    e_debounce_levels_t e_debounce_get_level(const s_debounce_t* ps_DB);

    /**
     * @brief Get and consume a pending rising edge event
     *
     * A rising edge event is generated when a valid low to high transition, has been detected by the debounce filter
     *
     * Once reported, the event is cleared and will not be returned again until a new validated rising edge occurs
     * @param ps_db Pointer to the debounce instance
     * @return Rising edge event status
     */
    e_debounce_events_t e_debounce_get_event_rising(s_debounce_t* ps_db);

    /**
     * @brief Get and consume a pending falling edge event
     *
     * A falling edge event is generated when a valid high to low transition, has been detected by the debounce filter
     *
     * Once reported, the event is cleared and will not be returned again until a new validated falling edge occurs
     * @param ps_db Pointer to the debounce instance
     * @return Falling edge event status
     */
    e_debounce_events_t e_debounce_get_event_falling(s_debounce_t* ps_db);

#endif /* DEBOUNCE_H */