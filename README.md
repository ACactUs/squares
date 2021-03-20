# Cquares
A simple command line evolution simulator written in C

## Building
Simply run `make` inside project directory.  
Requires ncurses library. Does not compile on Windows

## Reason
This is a fun little experiment with genetic algorithms and functional programming. I aimed to create an agile plugin-like projet, where data and code are separated as much as possible and program behavior is defined mostly by data. It depends only on ncurses, as I wanted to challenge myself.

## Mechanics
Plane is populated with rectangles, each has personal stats and inheritable traits, which it passes down the next generation after recombinating with another rectangle. Rectangles collide and fight for resources with each other. 
Each rectangle has traits and actions which are placed into action slots. Actions is a set of predefined functions and traits is just an array of floats which serve as parameters of these functions. Control loop uses a jump table to select an action. Over time the system evolves and most rectagnles inherit reasonable behaviour.

#### The plane
![image](https://user-images.githubusercontent.com/25302233/111864594-838aaa00-8994-11eb-9b99-59653aeee2cb.png)

#### Rectangle's traits
![image](https://user-images.githubusercontent.com/25302233/111864612-9e5d1e80-8994-11eb-9e73-8e3c0349b17b.png)

#### In-game menus are written in functional style, they are higher-order functions, which provides agility
![image](https://user-images.githubusercontent.com/25302233/111864672-f431c680-8994-11eb-8c38-5233969a2f12.png)

#### Here is rectangles right-side ancestor
![image](https://user-images.githubusercontent.com/25302233/111864652-e11ef680-8994-11eb-93e1-b029d38dfe68.png)

#### Check it out!
