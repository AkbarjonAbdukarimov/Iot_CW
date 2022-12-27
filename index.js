
const mqtt = require('mqtt')

const host = 'n5081226.us-east-1.emqx.cloud'
const p = '15323'
const clientId = `mqtt_${Math.random().toString(16).slice(3)}`

const connectUrl = `mqtt://${host}:${p}`
const client = mqtt.connect(connectUrl, {
  clientId,
  clean: true,
  connectTimeout: 4000,
  username: 'test',
  password: 'test1234',
  reconnectPeriod: 1000,
})

const topic = 'topic_hum'

const express = require('express');
const app = express();
const PORT = 4000;

//New imports
const http = require('http').Server(app);
const cors = require('cors');

app.use(cors());
const socketIO = require('socket.io')(http, {
    cors: {
        origin: "http://localhost:3000"
    }
});
client.on('connect', () => {
  console.log('Connected')
  client.subscribe([topic], () => {
    console.log(`Subscribe to topic '${topic}'`)
  })
  
  
})

//Add this before the app.get() block
socketIO.on('connection', (socket) => {
  const fire="fire"
  client.on('message', (topic, payload) => {
    const msg= payload.toString()
    socket.on('message', (data) => {
      socketIO.emit('messageResponse', msg);
    });
  });
  client.subscribe(['fire'], () => {
    console.log(`Subscribe to topic fire`);
    client.on('message', (fire, payload) => {
    const msg= payload.toString()
    socket.on('message', (data) => {
      
    });
    console.log('fire msg',msg)
  });
  })
   
 


  
  socket.on('message', (data) => {
   console.log(data.state);
    client.publish(data.to, data.state.toString(), (error) => {
      
      if (error) {
        console.error(error)
      }
  })
    
  });
  //sends the message to all the users on the server


  socket.on('disconnect', () => {
    console.log('🔥: A user disconnected');
  });
});




app.get('/api', (req, res) => {
  res.json({
    message: 'Hello world',
  });
});

http.listen(PORT, () => {
  console.log(`Server listening on ${PORT}`);
});