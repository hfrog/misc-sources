//
// simple http responder
//
// $Id: index.js,v 1.1 2016/11/21 11:54:41 root Exp $
// 
const http = require('http');
const util = require('util');
const os   = require('os');
const fs   = require('fs');
const dateFormat = require('dateformat');

var requests = 0;

var handleRequest = function(request, response) {
  response.setHeader('Content-Type', 'text/plain');
  response.writeHead(200);

  // fs.access(path, (err) => { ... })
  var resp = util.format('[%s] Req #%d from %s:%s processed by %s %s:%s',
        dateFormat(new Date(), "mmm dd HH:MM:ss"),
        requests++,
        request.connection.remoteAddress, request.connection.remotePort,
        process.env.HOSTNAME || os.hostname(),
        request.connection.localAddress, request.connection.localPort);

  response.end(resp + '\n');
  console.log(resp);
}

var www = http.createServer(handleRequest);

www.listen(80,function () {
  var startTime = new Date();

  console.log ("Started at:", startTime);
});
