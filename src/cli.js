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
//print ('Hello, World!');
//
//print(readline());

var i = 10;

MyObject = {
  x: 12,
  y: 'Value of x is ',
  foo: function ()
  {
    return this.y + this.x;
  },
  komplete: function (some)
  {
    //print("komplete " + some);

    if(some)
    {
      return "something";
    }
    else
    {
      print("\nhelp\nexit\ncat\nsystem\npipe");
    }
  },
  compute: function (some)
  {
    print(i);
    print("compute " + some);
    i = 11;

    var userinput = some.split(" ");

    switch(userinput[0])
    {
      case "exit":
        running = false;
        return 300;
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
    return 25;
  }
}

// function komplete()
// {
//   printf("KOMPLETE2");
// }

// var files = ["one.json", "two.json", "three.json"];
//
// var contents = [];
//
// //files.forEach((item, i) => {
// //  print("hello");
// //});
//
// for (var i = 0; i < files.length; i++) {
//   contents[i] = cat(files[i]);
// }
//
// for (var i = 0; i < contents.length; i++) {
//   //print(contents[i]);
// }

// var running = true;
// //
// while(running)
// {
  // var userinput = readline().split(" ");
  //
  // switch(userinput[0])
  // {
  //   case "exit":
  //     running = false;
  //   break;
  //   case "help":
  //     print("help: help\nexit: exit\ncat: cat\nsystem: system\npipe: pipe\n");
  //   break;
  //   case "kitty":
  //     print("meow");
  //   break;
  //   case "cat":
  //     print(cat(userinput[1]));
  //   break;
  //   case "json":
  //     var myjson = JSON.parse(cat(userinput[1]));
  //     print(myjson.something);
  //   break;
  //   case "echo":
  //     print(userinput[1]);
  //   break;
  //   case "system":
  //     system(userinput[1]);
  //   break;
  //   case "pipe":
  //     print(pipe(userinput[1]));
  //   break;
  //   default:
  //     print(userinput[0]);
  // }
// }
