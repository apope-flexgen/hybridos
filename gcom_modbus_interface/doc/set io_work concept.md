
### How we aggregate and create io_work structures from incominf fims set ( and get) messages 
Overview on aggregating the set objects 
```


 if (io_fims->method_view == "set")
    {

        std::vector<std::shared_ptr<cfg::map_struct>>io_map_vec;
        std::vector<std::shared_ptr<IO_Work>>io_work_vec;

        if(io_fims->uri_req.uri_vec.size() > 3 )
        {

            {
                //bool debug = true;
                std::shared_ptr<cfg::map_struct> var_result;
    
                // have to look for _disable / _enable  / _force /_unforce

                //std::cout << "  looking for var  " << "\n";
                auto single_var = test_findMapVar(var_result, myCfg, io_fims->uri_req.uri_vec, "dummy");
                if(single_var) 
                {
                    //std::cout << " single var found " << var_result->id << "\n";

                    var_result->set_value = io_fims->anyBody;
                    io_map_vec.emplace_back(var_result);

                }

            }
        }
        else if(io_fims->uri_req.uri_vec.size() <= 3 )
        {
            //std::cout << "We got a  multi set method; anyBody " << io_fims->anyBody.type().name() << " ";
            if (io_fims->anyBody.type() == typeid(std::map<std::string, std::any>)) 
            {
                auto mval = std::any_cast<std::map<std::string, std::any>>(io_fims->anyBody);
                for (auto m : mval) 
                {
                    bool debug = true;
                    std::shared_ptr<cfg::map_struct> var_result;

                    // have to look for _disable / _enable  / _force /_unforce
                    //std::cout << "  looking for var  " << "\n";
                    auto multi_var = test_findMapVar(var_result, myCfg, io_fims->uri_req.uri_vec, m.first);
                    if(multi_var) 
                    {
                        var_result->set_value = m.second;
                        io_map_vec.emplace_back(var_result);

                    }
                }
            }
        }

        // sort and collect io_work items
        check_work_items(io_work_vec, io_map_vec, myCfg, "set", true);        
        if (io_work_vec.size()> 0)
        {
            int idx = 0;
            for (auto io : io_work_vec)
            {
                // TODO put the set_value (any) structure into the io_work buffer for each map object in the io_work
                ///uint64_t getanyuval(std::shared_ptr<cfg::map_struct>item, std::any val, uint16_t*regs16);
                //uint8_t getanybval(std::shared_ptr<cfg::map_struct>item, std::any val, uint8_t*regs8);

                std::cout << " work item  [" << idx<< "] found , start " << io->offset<< " num :" << io->num_registers << std::endl;
                // send to io_threads
                setWork (*io);

                ++idx;
            }

        }
    }
    ```
    