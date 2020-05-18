import * as jsonpointer from "jsonpointer.js";

var obj = { foo: 1, bar: { baz: 2}, qux: [3, 4, 5]};

print(jsonpointer.get(obj, '/foo'));
