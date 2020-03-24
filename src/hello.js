// var myobj = {one:1, two:2};
//
// print(myobj.one + myobj.two);
//
// print (my_var);
//
// joke();
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
  var userinput = readline();
  print(userinput);

  switch(userinput)
  {
    case "exit":
      running = false;
    break;
    default:
  }
}
