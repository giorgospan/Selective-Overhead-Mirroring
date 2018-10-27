# Selective Overhead Mirroring

This is the third assignment of "Unix Systems Programming" course (spring 2017).

The main goal of this assignment is to create a *selective overhead mirroring application* , which will be capable of copying multiple directories from a remote Content Server to a selected machine (Mirror Server). This mirroring system consists of a series of web-based programs that all together achieve the desired file transfer. Individual programs should use threads to achieve data transfer.

Application comprises three main programs:
  * **MirrorInitiator :** Connects to MirrorServer program via a network and sends the location of the remote Content Servers as well as the name of the directories stored in them , which will be mirrored. A delay tolerance parameter is sent, too.
  * **MirrorServer :** Upon receiving the required information from Initiator, it starts retrieving the files from the remote Servers using a bunch of threads, called MirrorManagers. Note that there is one MirrorManager thread per Content Server. Afterwards, MirrorServer stores the received files to his local storage with the help of a fixed number of Worker threads.
  * **ContentServer :** This program will be run in the remote devices. It is responsible for serving the mirror-requests submitted by the MirrorServer with respect to the available directories, as well as the transfer of every individual file.


The following picture shows how the application works
![img not found](https://github.com/giorgospan/SelectiveOverheadMirroring/blob/master/figure.png "Figure")


