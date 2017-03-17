# Pi_296_Cluster

### Project Logistics

- Team meeting: 5:30PM Mon, Wed @ Siebel Basement
- Mentor meeting: 5:30PM Fri @ Siebel Basement

### Todo List

1. ***Put implementation details here!***
2. ***Put development timeline here!***
3. Learn about networking ([Beej](http://beej.us/guide/bgnet/), [Wikibook](https://github.com/angrave/SystemProgramming/wiki))
4. Implement basic networking, test on VMs
5. Wire up Raspberry Pis and set up OS

### Implementation
(Added by Andrew, not agreed upon yet)
   Stage 1: Implement 3 or 4 Raspberry Pi's, loaded with Raspbian, to communicate in a master->worker relationship to complete various task.
   (when hardware arrives) Stage 2: Set up 6 Raspberry Pi's in a complete graph fashion, carrying over the functionality from stage one, but adding node->node communications instead of a dedicated master, with fault tolerance built in.
### Timeline
(Added by Andrew, not agreed upon yet)
- Week1:
   1. Put together a basic cluster of 3 nodes, and achieve "plug and play."
   2. Figure out basic networking using Cs241 VM's.
   3. Figure out how to transfer networking code to cluster, and make it work.
   4. (Reach) Develop benchmark tests.

- Week2:
   1. Develop ability to send data between nodes.
   2. Develop continuous server code on master.
   3. Have the master send bench test to workers, and them send back the results.
   4. (Reach) Work on library for using GPIO pins with a breadboard.

- Week3:
   1. Develop error checking of expected return values from workers.
   2. Figure out how to make a task scheduler.
   3. Standardize ways to make nodes send back bad results when commanded.

- Week4:
   1. Finish any leftover from Week 3.
   2. Standardize way to give task to master while server running.
   3. Enable the master to send task out based on "priorities".
   4. (Reach) Have the master resend task to network when it gets bad data.

- Week5:
   1. Work on fleshing out status checks and expected run time.
   2. Develop handing out node priority for master at run time.
   3. Enable the network nodes to all communicate with one another, instead of just the master.

- Week6:
   1. Finish any uncompleted work from week 5.
   2. Figure out how to integrate a previously taken down node into the network workforce.
   3. Figure out error handling of failed nodes with assigned work.

- Week7:
   1. Unplanned.

- Week8:
   1. Unplanned.

### The Structure:

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
                    -> wait for task from master
                    -> create new thread
                    -> join thread after receiving flag from subthread

                       (subthread) -> execute task
                                   -> call networkManager.c -> sendResponse -> master
                                   -> exit

### How The Network Handles Itself

- Situation one: System startup  
    This means the default master node, or server, has not started yet. Thus, the server will being starting up, using port 9001 by default as the accepting slave nodes port. After starting up with no errors, the server will listen until it hears a connect, creating a separate port for that connection, and waiting for the message with bits (0,3) stating, "Do you receive me?". The server will respond "Yes, I received you.", The slave will then send a message with bits(0,6) with message "Ready for work.", and remain waiting until the server sends it some message.

- Situation two: Server sending a task to a slave  
	If the master is sending a task with data needing to be available in a node, it will handle the data sent before sending the exec file. This includes any args needed for the exec file. After the slave has everything needed to run the exec, the master will send a message with bits (0,4) and the raw binary file to be saved and exec on the slave. Subsequent messages will contain the message bits. After the Master has finished sending the binary file, it will send a message with bits (0,3) stating "Are you running?". The slave, after exec the program, will send a message with bits (0,6) "Running.". At this point, the server 'should' continuously ping the slave for status updates.

- Situation three: Server sending data to a slave  
	For any data file sent to a slave from a standby situation, the message bits (0,5) will be followed by 64 bytes for a file name. The following data will be placed into that file opened on the slave. Any subsequent messages containing more data will be with the bits (0,5) followed by the data, not using the 64bytes space for the name. After the data is done sending, the server will send a message (0,3) stating "Are you ok?". The slave will send a message (0,6) "Ready for work." and wait.

- Situation four: Slave finished with task  
	Whenever a master message of (0,3) "Are you running?" is received after a slave is finished running, the slave will send back a message (0,6) "Finished running." The master will either send a (0,3) message of "I received you.", in which the slave will send (0,6) "Ready for work." and wait. If any data is needed from the slave, one should follow the "Getting data from slave".

- Situation five: Getting data from slave  
	The master will send a (0,3) message asking "Send me the file:", followed by the file name to send. The slave will then send (0,5) message followed by 64 bytes of the file name, followed by data, following the convention for sending data to a slave. When all the data is send, the slave will send the (0,3) message "Ready for work.".
