# Galaxy_Coders_2023
Galaxy Coders AI base for starter soccer simulation leage


    if(!state_file.is_open() || !action_file.is_open()
          || !reward_file.is_open() || !next_state_file.is_open() || !done_file.is_open())
    {
        std::ofstream state_file(  path +"memory/" + part + "/" + "states/" + name + "_states.csv" );
        std::ofstream action_file( path +"memory/" + part + "/" + "actions/" + name + "_actions.csv");
        std::ofstream reward_file( path + "memory/" + part + "/"+ "rewards/" + name + "_rewards.csv");
        std::ofstream next_state_file( path +"memory/" + part + "/" + "next_states/" + name + "_next_states.csv");
        std::ofstream done_file( path + "memory/" + part + "/" + "done/" + name + "_done_memory.csv");
    }
    
