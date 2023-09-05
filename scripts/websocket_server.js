const WebSocketServer = require('ws').WebSocketServer

const PORT = 8022

console.infoCopy = console.info.bind(console)
console.info = function(data) {
    this.infoCopy('[' + new Date().toUTCString() + '] ', data)
}


console.info(`Starting Simple WebSocketServer on Port:${PORT}`)

const wss = new WebSocketServer({ port: PORT, host: '0.0.0.0' })

wss.on('connection', function connection(ws) {
  console.info('New Connection')
  
  ws.on('error', console.error)
  ws.on('message', function message(data) {
    console.log('received: %s', data)
  })
 
  ws.on('close', function clear() {
    console.info('Closed Connection')
  })
  
  ws.send('Welcome to Sate Playground')
  
})

const interval = setInterval(function ping() {
  console.info('Sending Time')
  wss.clients.forEach(function each(ws) {
    ws.send(`The time is ${new Date().toUTCString()}`)
  });
}, 5000)

wss.on('close', function close() {
  clearInterval(interval);
});
