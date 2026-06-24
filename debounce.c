/**********************************************************************************
 *  _____ ___  _   _      ___  _____   _____ _    ___  ___ __  __ ___ _  _ _____ 
 * |_   _|   \| | | |    |   \| __\ \ / / __| |  / _ \| _ \  \/  | __| \| |_   _|
 *   | | | |) | |_| |    | |) | _| \ V /| _|| |_| (_) |  _/ |\/| | _|| .` | | |  
 *   |_| |___/ \___/     |___/|___| \_/ |___|____\___/|_| |_|  |_|___|_|\_| |_|  
 *
 **********************************************************************************/
/**
 * @file    debounce.c
 * @brief   Software debounce filter for digital inputs
 * @author  Théo DUPERRET
 * @see     <a href="https://github.com/tduperret">GitHub</a>
 * @see     <a href="https://gitlab.com/tduperret">GitLab</a>
 * @see     <a href="https://www.linkedin.com/in/duperret-theo">LinkedIn</a>
 * @date    24/06/2026 - Last Edit : 24/06/2026
 */

#include "debounce.h"

#include <stddef.h>

e_debounce_status_t e_debounce_init(s_debounce_t* ps_db, uint32_t ui32_id, b_debounce_read_func_t read, void *context, uint8_t ui8_cycles)
{
    if(ps_db == NULL) 
    {
        return e_DEBOUNCE_STATUS_ERROR_INVALID_INSTANCE;
    }
    
    if(read == NULL)
    {
        return e_DEBOUNCE_STATUS_ERROR_INVALID_READ_FUNCTION;
    }

    ps_db->ui32_id = ui32_id;
    ps_db->read = read;
    ps_db->context = context;

    if(ui8_cycles == 0)
    {
        ui8_cycles = 1;
    }

    if(ui8_cycles > 31)
    {
        ui8_cycles = 31;
    }

    ps_db->ui8_cycles = ui8_cycles;
    ps_db->ui32_mask = (1UL << ui8_cycles) - 1UL;

    return e_debounce_reset(ps_db);
}

e_debounce_status_t e_debounce_reset(s_debounce_t* ps_db)
{
    if(ps_db == NULL)
    {
        return e_DEBOUNCE_STATUS_ERROR_INVALID_INSTANCE;
    }

    ps_db->ui32_history = 0;
    ps_db->b_level = false;
    ps_db->b_initialized = false;

    ps_db->b_rising_event = false;
    ps_db->b_falling_event = false;

    return e_DEBOUNCE_STATUS_OK;
}

e_debounce_status_t e_debounce_update(s_debounce_t* ps_db)
{
    if(ps_db == NULL)
    {
        return e_DEBOUNCE_STATUS_ERROR_INVALID_INSTANCE;
    }

    ps_db->ui32_history = (ps_db->ui32_history << 1) | (ps_db->read(ps_db->context) ? 1UL : 0UL);

    if((ps_db->ui32_history & ps_db->ui32_mask) == 0)
    {
        if(!ps_db->b_initialized)
        {
            ps_db->b_level = false;
            ps_db->b_initialized = true;
        }
        else if(ps_db->b_level != false)
        {
            ps_db->b_level = false;
            ps_db->b_falling_event = true;
        }
    }
    else if((ps_db->ui32_history & ps_db->ui32_mask) == ps_db->ui32_mask)
    {
        if(!ps_db->b_initialized)
        {
            ps_db->b_level = true;
            ps_db->b_initialized = true;
        }
        else if(ps_db->b_level != true)
        {
            ps_db->b_level = true;
            ps_db->b_rising_event = true;
        }
    }

    return e_DEBOUNCE_STATUS_OK;
}

e_debounce_levels_t e_debounce_get_level(const s_debounce_t* ps_DB)
{
    if(ps_DB == NULL)
    {
        return e_DEBOUNCE_LEVEL_UNKNOWN;
    }

    if(!ps_DB->b_initialized)
    {
        return e_DEBOUNCE_LEVEL_UNKNOWN;
    }

    if(ps_DB->b_level)
    {
        return e_DEBOUNCE_LEVEL_HIGH;
    }
    else
    {
        return e_DEBOUNCE_LEVEL_LOW;
    }
}

e_debounce_events_t e_debounce_get_event_rising(s_debounce_t* ps_db)
{
    if(ps_db == NULL)
    {
        return e_DEBOUNCE_EVENT_UNKNOWN;
    }

    if(ps_db->b_rising_event)
    {
        ps_db->b_rising_event = false;
        return e_DEBOUNCE_EVENT_PENDING;
    }

    return e_DEBOUNCE_EVENT_NONE;
}

e_debounce_events_t e_debounce_get_event_falling(s_debounce_t* ps_db)
{
    if(ps_db == NULL)
    {
        return e_DEBOUNCE_EVENT_UNKNOWN;
    }

    if(ps_db->b_falling_event)
    {
        ps_db->b_falling_event = false;
        return e_DEBOUNCE_EVENT_PENDING;
    }

    return e_DEBOUNCE_EVENT_NONE;
}
