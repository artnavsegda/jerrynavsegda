// var myobj = {one:1, two:2};
//
// print(myobj.one + myobj.two);
//
// print (my_var);
//
//joke();
//
// var strda = joke("hello.txt");
//
// print ("return " + strda);
//
// myjson = JSON.parse(strda);
//
// print(myjson);
// print(myjson.something);
//
// var str = MyObject.myFunc ();
// print (str);
//
// print ('Hello from JS with ext!');
//
//print(readline());

var running = true;
//
while(running)
{
  var userinput = readline().split(" ");

  switch(userinput[0])
  {
    case "exit":
      running = false;
    break;
    case "help":
      print("help: help\nexit: exit");
    break;
    case "kitty":
      print("meow");
    break;
    case "cat":
      print(cat(userinput[1]));
    break;
    case "json":
      var myjson = JSON.parse(cat(userinput[1]));
      print(myjson.something);
    break;
    case "echo":
      print(userinput[1]);
    break;
    case "system":
      system(userinput[1]);
    break;
    default:
      print(userinput[0]);
  }
}
