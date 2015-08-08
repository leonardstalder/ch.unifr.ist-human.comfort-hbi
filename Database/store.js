var mqtt=require('mqtt')  
var mongodb=require('mongodb');  
var mongodbClient=mongodb.MongoClient;  
var mongodbURI='mongodb://username:password@server.mongohq.com:port/database'  
var deviceRoot="demo/device/"  
var collection,client;  

mongodbClient.connect(mongodbURI,setupCollection);

function setupCollection(err,db) {  
  if(err) throw err;
  collection=db.collection("test_mqtt");
  client=mqtt.createClient(1883,'localhost')
  client.subscribe(deviceRoot+"+")
  client.on('message', insertEvent);
}


function insertEvent(topic,payload) {  
  var key=topic.replace(deviceRoot,'');
  
  
  collection.update(  
  { _id:key },
  { $push: { events: { event: { value:payload, when:new Date() } } } },
  { upsert:true },
  function(err,docs) {
    if(err) { console.log("Insert fail"); } // Improve error handling
  }
  )
}