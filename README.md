# Selective Overhead Mirroring


## About

The is an implementation of a *selective-overhead mirroring application* , which is capable of copying multiple directories from a remote machine (Content Server) to another selected machine (Mirror Server). This mirroring system consists of a series of web-based programs that work together in order to achieve the desired file transfer. Individual programs use threads to achieve data transfer.

## How it works

Application comprises three main programs:

  * **MirrorInitiator :** Connects to MirrorServer program via a network and sends the location of the remote Content Servers as well as the name of the directories stored in them , which will be mirrored. A delay tolerance parameter is sent, too.

  * **MirrorServer :** Upon receiving the required information from Initiator, it starts retrieving the files from the remote Servers using a bunch of threads, called MirrorManagers. Note that there is one MirrorManager thread per Content Server. Afterwards, MirrorServer stores the received files to his local storage with the help of a fixed number of Worker threads.

  * **ContentServer :** This program will run in the remote devices. It is responsible for serving the mirror-requests submitted by the MirrorServer with respect to the available directories, as well as the transfer of every individual file.


The following image illustrates how the mirroring system works
![img not found](https://github.com/giorgospan/SelectiveOverheadMirroring/blob/master/figure.png "Figure")


## Usage



