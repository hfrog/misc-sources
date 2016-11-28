//
// simple http responder node.js app
//
// $Id: index.js,v 1.3 2016/11/21 13:00:55 root Exp root $
//
var http       = require('http');
var util       = require('util');
var os         = require('os');
var fs         = require('fs');
var express    = require('express');
var dateFormat = require('dateformat');

//
// TCP port to listen to
//
var port = 80;

//
// if badfile exists, server will return 404 on / request
//
var badfile = 'bad';

//
// code begins
//

//
// create express app instance
//
var app = express();

//
// / handler
//
app.get('/', function(req, res, next) {
  fs.access(badfile, (err) => {
    if (err) {
      // no bad file exists
      var resp = log_str(req) + ' success';
      res.send(resp + '\n');
      console.log(resp);
    } else {
      // there is the bad file exists,
      // pass request to the next (default) handler
      next();
    }
  });
});

//
// default handler
//
app.use(function(req, res, next) {
  var resp = log_str(req) + ' fail';
  console.log(resp);
  next();
});

//
// http server initialization
//
var server = http.createServer(app);
server.listen(port);
server.on('error', onError);
server.on('listening', onListening);

//
// functions only below this line
//

//
// utility functions
//
function my_date() {
  return dateFormat(new Date(), "mmm dd HH:MM:ss");
}

function log_str(req) {
  // initialize request counter if needed
  if (typeof log_str.requests == 'undefined') {
    log_str.requests = 0;
  }
  return util.format('[%s] Req #%d from %s:%s processed by %s %s:%s %s %s',
        my_date(), log_str.requests++,
        req.connection.remoteAddress, req.connection.remotePort,
        process.env.HOSTNAME || os.hostname(),
        req.connection.localAddress, req.connection.localPort,
        req.method, req.path);
}

//
// server handlers
//
function onError(error) {
  if (error.syscall !== 'listen') {
    throw error;
  }

  var bind = typeof port === 'string'
    ? 'Pipe ' + port
    : 'Port ' + port;

  // handle specific listen errors with friendly messages
  switch (error.code) {
    case 'EACCES':
      console.error(bind + ' requires elevated privileges');
      process.exit(1);
      break;
    case 'EADDRINUSE':
      console.error(bind + ' is already in use');
      process.exit(1);
      break;
    default:
      throw error;
  }
}

function onListening() {
  var addr = server.address();
  var bind = typeof addr === 'string'
    ? 'pipe ' + addr
    : 'address ' + addr.address + ' port ' + addr.port;
  console.log('Started at', my_date(), 'listening on', bind);
}

