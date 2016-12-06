# ORM_MYSQL
Mysql ORM (Object Relation Mapping)

# a C++ Orm for mysql #
:relaxed:steal many many ideas and codes from this [cool man](https://github.com/BOT-Man-JL/ )'s :hurtrealbad: [sqlite orm](https://github.com/BOT-Man-JL/ORM-Lite)

## 1 Preparation  ##
well,firstly you should have installed your mysql.

1.1 Install C connector for mysql 
----------------------------------

You can install it from [ official website](https://dev.mysql.com/downloads/connector/c/) 

1.2 Add and Include orm_mysql.h with your source files
------------------------------------------------------

1.3 Add compile option "-lmysqlclient"
--------------------------------------

if you use makefile tool or gcc,just add it. Or if you are in windows by vs, you should include a dynamic library called "mysqlclient.lib" (I guess it's called that.....)

in Qt, you should add "QMAKE_LIBS+= -lmysqlclient" in your pro file.


## 2 Use it  ##
2.1 Connect your database
-------------------------

firstly,you should add

```C++
using namespace ORM_MYSQL_THIEF
```
and then you should add some codes at your class, just like this:

```C++
// add this line 
using namespace ORM_MYSQL_THIEF;

class MyClass
{

// this line are critical, first "serverDB" is your table name
// And id ... score are column fields that you want to add to your database
ORMAP_MYSQL(serverDB,id,name,level,score)

public:

MyClass(std::string idr,std::string namer,int levels,float score)
            :id(idr),name(namer),level(levels),score(score)
    {}

    std::string id;
    std::string name;
    int level;
    float score;
};
```

Then you can connect your mysql by just a line code:

```C++
 ORMapper mapper("localhost","username","password","database",port);
```



2.2 create,delete,insert,update,query,count
-------------------------------------------

This orm (:laughing: i think i can say it's called that...) just implements few operations and if you are interested in it, you can change it.

```c++
/////////// You can create your class like this.
 std::vector<MyClass> listClassObject;
    MyClass helper("000","yyy",0,0);
    MyClass wodner1("111","thief",999,56.4);
    MyClass wodner2("222","fly",88,54.2);
    MyClass wodner3("333","universe",5,45.9);
    MyClass wodner4("444","keybord",34,99.9);

    listClassObject.push_back(wodner1);
    listClassObject.push_back(wodner2);
    listClassObject.push_back(wodner3);
    listClassObject.push_back(wodner4);

////////// Then you can _DO_ :
        /* two way to create a table. */
        mapper.createTbl(helper);   //create a table by info in wodner1
//      | id | name | level | score |

        mapper.createTbl(wodner1,1,"number");  // add a "auto_increment" field "number" to your table
//      | id | name | level | score | number |   

        /* insert  classObject */
        mapper.insert(wodner1);   // insert one 
        mapper.insertRange(listClassObject);   // insert a vector
        
        /* select and query */
        mapper.select(helper,Exp("name,id")).toVector();  //select name,id 
        mapper.select(helper).toVector();
        
        auto resultA=mapper.select(helper) 
                      .query()
                      .where(Exp("level")<78)
                      .toVector();
                      
        auto resultB=mapper.query(helper)      // no select, will select *
                     .where(Exp("level")<90)       // also .where(Exp("name")=="thief")
                     .limit(2)
                     .offset(0)
                     .toVector();
// when you want to create a table, select or query, you need to pass a "helper" classObject.
// you can find we can use _Exp_ to wrap your fields names, then we can create _select_ fields (like "name,id")
_where_ (like "level")for querying.

// query and select will get a two-dimensional vector whose structure you can get value like this:
        for(auto h:resultB)
        {
            // resultB is a two-dimensional vector, h is a vector
            // now h[0] is a string for "id", h[1] is a string for "name", h[2] is a int for "level", h[3] is a float for "score"
            MyClass lol(h[0],h[1],h[2],h[3]);
            lol.printItself();
        }
        
        /* count */
        int counter=mapper.query(helper).where(Exp("level")<88).count(); // return a number for your query

    
```

## 3 welcome to improve it~ ##
:laughing: you can find many details which can be modified. 
