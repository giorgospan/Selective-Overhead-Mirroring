# Selective Overhead Mirroring


## About

The is an implementation of a *selective-overhead mirroring application* , capable of mirroring multiple directories from a remote machine (Content Server) to another selected machine (Mirror Server). This mirroring system consists of a series of web-based programs that work together in order to achieve the desired file transfer. Individual programs use threads to achieve data transfer.

## How it works

Application comprises three main programs:

* **MirrorInitiator :**

  Connects to MirrorServer program via a network and sends the location of the remote Content Servers as well as the name of the directories stored in them , which will be mirrored. A delay tolerance parameter is sent, too.

* **MirrorServer :**

  Upon receiving the required information from Initiator, it starts retrieving the files from the remote Servers using a bunch of threads, called MirrorManagers. Note that there is one MirrorManager thread per Content Server. Afterwards, MirrorServer stores the received files to his local storage with the help of a fixed number of Worker threads.

   * **MirrorManager Threads** :

     MirrorManager threads are created as soon as MirrorServer receives a mirror request from the initiator. They work completely in parallel, each one communicating with a different ContentServer in the following fashion:

     1. MirrorManager thread sends a *LIST* request to its respective ContentServer providing it with delay, which will be used later by the ContentServer during data transfer.

     2. ContentServer responds back with a list of directories and files available for mirroring to the MirrorServer. The manager thread stores this list in a limited-size queue, shared by the worker threads.

   * **Worker Threads** :

      They run in parallel and are responsible for mirroring and storing the files in MirrorServer. More precisely, worker's task can be described by the following steps:

      1. Worker thread extracts a directory/file from the buffer(queue).

      2. It then sends a FETCH request to the ContentServer serving this specific directory/file.

      3. ContentServer sends the requested directory/file after having added the appropriate delay (overhead).

      4. Worker receives the directory/file and stores it in MirrorServer's machine.

  * **Shared Queue** :

     MirrorManagers and Workers act as producers and consumers respectively. Thus, in order to avoid race conditions occurred during the concurrent manipulation of the buffer(queue) by MirrorManager threads and Worker threads, we make use of condition variables. Busy-waiting is not option due to its inefficiency.


* **ContentServer :**

  This program will run in the remote devices. It is responsible for serving the mirror-requests submitted by the MirrorServer with respect to the available directories, as well as the transfer of every individual file.


The following image illustrates how the mirroring system works
![img not found](https://github.com/giorgospan/SelectiveOverheadMirroring/blob/master/figure.png "Figure")


## Usage

### Content Server

* `make content`

* `cd build`

* `./ContentServer -p <port> -d <dirorfilename>`

  * `port` : port number that ContentServer is listening to

  * `dirorfilename` : directory or file available for mirroring

### Mirror Server

* `make mirror`

* `cd build`

* `./MirrorServer -p <port> -m <dirname> -w <threadnum>`

  * `port` : port number that MirrorServer is listening to

  * `dirname` : directory for storing the mirrored directories/files

  * `threadnum` : number of worker threads

### Mirror Initiator

* `make init`

* `cd build`

* `./MirrorInitiator -n <MirrorServerAddress> -p <MirrorServerPort> \\
-s <ContentServerAddress1:ContentServerPort1:dirorfile1:delay1, \\
ContentServerAddress2:ContentServerPort2:dirorfile2:delay2, ...>`

  * `MirrorServerAddress` : IP Address or hostname of the machine running the MirrorServer program

  * `MirrorServerPort` : port number that MirrorServer is listening to

  * `ContentServerAddressX` : IP Address or hostname of the machine running the ContentServerX program

  * `ContentServerPortX` : port number ContentServerX is listening to

  * `dirorfileX` : directory or file available for mirroring from ContentServerX

  * `delayX` : seconds added by ContentServerX before sending a directory or file


#### Testing on localhost

`./ContentServer -p 9001 -d ./content1`

`./ContentServer -p 9002 -d ./content2`

`./MirrorServer -p 9000 -m ./mirrored -w 5`

`./MirrorInitiator -n localhost -p 9000 -s localhost:9001:dir/in/content1:0,localhost:9002:file_in_content2:1`

#### Note

Make sure ContentServer and MirrorServer are running before you start the MirrorInitiator program




