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

var files = ["one.json", "two.json", "three.json"];

var contents = [];

//files.forEach((item, i) => {
//  print("hello");
//});

for (var i = 0; i < files.length; i++) {
  contents[i] = cat(files[i]);
}

for (var i = 0; i < contents.length; i++) {
  print(contents[i]);
}

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
      print("help: help\nexit: exit\ncat: cat\nsystem: system\npipe: pipe\n");
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
    case "pipe":
      print(pipe(userinput[1]));
    break;
    default:
      print(userinput[0]);
  }
}
