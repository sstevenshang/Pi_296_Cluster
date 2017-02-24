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
   (when hardware arrives) Stage 2: Set up 6 Raspberry Pi's in a complete graph fashion, carrying over the functionality from stage one, but adding node->node commuications instead of a dedicated master, with fault tolerance built in.
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

-Week3:
   1. Develop error checking of expected return values from workers.
   2. Figure out how to make a task scheduler.
   3. Standardize ways to make nodes send back bad results when commmanded.

-Week4:
   1. Finish any leftover from Week 3.
   2. Standardize way to give task to master while server running.
   3. Enable the master to send task out based on "priorities".
   4. (Reach) Have the master resend task to network when it gets bad data.

-Week5:
   1. Work on fleshing out status checks and expected run time.
   2. Develop handing out node priority for master at run time.
   3. Enable the network nodes to all comunicate with one another, instead of just the master.

-Week6:
   1. Finish any uncompleted work from week 5.
   2. Figure out how to integrate a previously taken down node into the network workforce.
   3. Figure out error handling of failed nodes with assigned work.

-Week7: 
   1. Unplanned.

-Week8:
   1. Unplanned.
