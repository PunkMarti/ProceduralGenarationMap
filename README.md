# ProceduralGenarationMap
 
Project about procedural generation of different types of maps and technique.

Maze :

You can use the AGDMMaze actor in a level to see the generation of the maze and you can modify the parameters inside the BP

We use the technique described on this page : https://www.astrolog.org/labyrnth/algrithm.htm

We use the "Perfect maze" technique by generating walls after walls as long as they "touch" something only at one of their end.

Dungeon :

You can use the AGDMDungeonManager in a level to see the generation of a dungeon and you can modify the parameters inside the BP

We use the technique described on this page : https://www.gamedeveloper.com/programming/procedural-dungeon-generation-algorithm

1.Generate rooms with random size/width
2.Separate them
3.Create a graph connecting them based on the Delaunay triangulation
4.Create the Hallways connecting the rooms
