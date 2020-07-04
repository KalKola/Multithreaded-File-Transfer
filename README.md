## Multithreaded Client Server File Transfer
![](transfer.png)

A C based program allowing files to be transfered from clients to a server. The server can handle multiple concurrent connections and transfer requests, and allows each user to specify the directory they wish to transfer the file to. Users may only transfer files to a directory of which they are a group member of, and additionally each transfered file can be traced back to the source user using the Linux user tag.

This C-based program allows files to be transfered from clients to a server. A **SOCK_STREAM** connection type is used to allow two-way communication through a byte stream, where confirmation messages can be sent back from the server on successful file transfer. The Server can handle multiple connections at once through the use of multithreading, where a seperate thread is created for each successful client connection.

On the client side the user may specify the directory they wish to transfer the file to. Users may only transfer files to a directory of which they are a group member of, which is enforced using a **getgrouplist()** to gather a list of all groups the user is a member of. These are passed to the server through an array, along with the related ID for each group. When the user specifies the directory they wish to transfer the file to, the group ID of that directory is compared to the previously sent array to see if it is contained within it. Following this, an appropriate message is transfered back to the client.

Finally, synchronisation for the multithreading was handled using a mutex lock, which locks access to shared resources when they are being used by a thread. 

A video demo of the program can be accessed <a href="https://www.youtube.com/watch?v=uWRpXaEMO8A">here.</a>
