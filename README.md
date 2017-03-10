# The Structure:

main.c -> Set Up Nodes -> Run server_main.c or master_main.c
          (node.c)
         
   master_main.c  -> (master.c functions)
                  -> keep heartbeat from servers
                  -> wait for uploaded tasks
                  -> distribute task   
                  -> create new thread 
                  -> join thread after receiving flag from subthread

                     (subthread) -> call networkManager.c -> sendTask -> server
                                 -> idle -> wait for response
                                         -> return response from thread
   
   server_main.c  -> (server.c functions)
                  -> keep heartbeat to master
                  -> wait for task from amster
                  -> create new thread
                  -> join thread after receiving flag from subthread

                     (subthread) -> execute task
                                 -> call networkManager.c -> sendResponse -> master
                                 -> exit