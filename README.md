#tidy

tidy is a programm that I wrote during my diploma thesis to validate my algorithms and to create result images.
The topic of my thesis was *Automatic image decomposition and the ordered display of its constituents*, and therefore tidy is capable of decomposing color images using *Mean Shift Segmentation* or *Watershed Segmentation* and rearranging the segments according to choosable criterias. 

##Known bugs
Large images can cause the application to crash, but images this large also are impractible, since the decomposition would take forever and a day. 
The rearrangement of the segments tends to crash in rare cases. This happens due to Qt messing up the bounding boxes of the used QGraphicsItems. The cause of this problem was found too late to fix it, and a solution would need a lot of coding to implement a alternative collision detection. 
