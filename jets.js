// Starts with node-add-on example: https://github.com/nodejs/node-addon-examples/tree/master/2_function_arguments/node-addon-api
var jets = require('./build/Debug/jets.node')

console.log('This should be eight:', jets.add(3, 5))
console.log('This should be 55:', jets.sum([1, 2, 3, 4, 5, 6, 7, 8, 9, 10]))
console.log('This should be OK:', jets.maketable('test', 'OK')['test'])
console.log('This should be Yes:', jets.forward({ 'test': 'Yes' })['test'])
console.log('This should be No:', jets.forward({ 'test': 'Yes', 'sub' : { 'test' : 'No' } })['sub']['test'])
let obj = jets.makeobject()
console.log('This should be an address to object', jets.getobjectaddress(obj))
console.log('This should be callback', jets.callback((s) => s))
