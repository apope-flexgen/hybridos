Old heartbeat logic.
p. wilshire 11_20_2023



watchdog_heartbeat: the heartbeat being sent from the Modbus or DNP3 server to its client, as defined in its configuration file. 
This component register must point to the watchdog_heartbeat register in assets.json for the watchdog feature to be functional.



component_connected : whether the component is connected according to the Modbus or DNP3 interface; will be true if the heartbeat is updating steadily and will not function if the Modbus or DNP3 client goes down. 
This component register must point to the component_connected register in assets.json for the watchdog feature to be functional.


We can use the tUpdate element in the io_point to detect that the server has gone.
we see the frozen status in which case we'll take component connected value as false.
NOTE we'll need a lock on this







read the heartbeat from holding or inputs
if we get errors then disable point
set component_connected false



            // NOTE(WALKER): This only reads the least significant word for heartbeat (whichever word is responsible for 0-65535) regardless of size (determined during load_config phase)
            if (comp_workspace.heartbeat_read_reg_type == Register_Types::Holding)
            {
                has_errno = modbus_read_registers(*my_lock.conn, comp_workspace.heartbeat_read_offset, 1, &comp_workspace.heartbeat_current_val) == -1;
            }
            else // input registers:
            {
                has_errno = modbus_read_input_registers(*my_lock.conn, comp_workspace.heartbeat_read_offset, 1, &comp_workspace.heartbeat_current_val) == -1;
            }
            if (has_errno)
            {
            const auto current_errno = errno;
            NEW_FPS_ERROR_PRINT("hardware #{} component {}'s heartbeat thread, error when reading modbus_registers, err = {}\n", hardware_id + 1, component_name, modbus_strerror(current_errno));
            if (current_errno == modbus_errno_disconnect || current_errno == modbus_errno_cant_connect)
            {
                comp_workspace.component_connected.store(false, std::memory_order_release); // set this component to be disconnected
                component_connected = false;
            }
            has_errno = false; // reset errno
        }
        If it was not connected and we got no errors then set component connected to true;

        if the component was connected and the value was not changed 
        send the not connected event.

        if we have a heartbeat write then add one and sent it out.
         modbus_set_response_timeout(*my_lock.conn, 0, ((comp_workspace.heartbeat_poll_frequency < modbus_default_timeout) ? comp_workspace.heartbeat_poll_frequency.count() : modbus_default_timeout.count()) * 1000);


else // we are disconnected just send it out like normal:
        {
            // NOTE(WALKER): Does this need "Timestamp"? -> is that really important on a disconnect? For now, no
            fmt::basic_memory_buffer<char, 50> send_buf;

            // this goes into the pub regardless of the rest of the data.

            fmt::format_to(std::back_inserter(send_buf), FMT_COMPILE(R"({{"modbus_heartbeat":{},"component_connected":false}})"), comp_workspace.heartbeat_current_val);

            if (!fims_gateway.send_pub(component_uri_view, std::string_view{send_buf.data(), send_buf.size()}))
            {
                NEW_FPS_ERROR_PRINT("can't send fims pub for hardware #{} component {}'s heartbeat thread, erroring out.\n", hardware_id + 1, component_name);
                return false;
            }
        }

        So if the heartbeat is a holding register we'll have to add it to the poll request. 
        have to sort out if we publsh it or not.


        So in the heartbeat task we 
        if we are already polling the register  poll_holdng or poll_input then no neeed to poll it.
        If not then run a poll at the heartbeat_poll_frequency.

        the normal response thread shpould handle it . we can treat as a get if its not part of a pub.
        in hte heartbeat handler we simply look for a value changed.
        and then send the hb on if required.
        