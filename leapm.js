var SamsungRemote = require('samsung-remote');
var remote=new SamsungRemote({
	ip:'192.168.2.9'
});
require('events').EventEmitter.prototype._maxListeners = 100;
var Leap = require('leapjs');
var five = require('johnny-five');
var a = false;
var b = false;
var c = false;
var clockwise=false;
var controller = new Leap.Controller();
//var IR = require("./IRremote.h");
var controller = Leap.loop({enableGestures: true}, function(frame){

	if(frame.valid && frame.gestures.length>0){
		frame.gestures.forEach(function(gesture){
			switch(gesture.type){
				case "circle":
					console.log("Circle Gesture");
					break;
				case "keyTap":
					console.log("Key Tap Gesture");
					break;
				case "swipe":
					console.log("Swipe Gesture");
					break;
			
			}
		
		});
	}

});


//var controllerOptions = {enableGestures:true};
/*
Leap.loop(controllerOptions,function(frame){

	if(frame.gestures.length>0){
		for(var i=0;i<frame.gestures.length;i++){
			var gesture =frame.gestures[i];
			if(gesture.type=="swipe"){
				var isHorizontal=Math.abs(gestures.direction[0])>Math.abs(gesture.direction[1]);
				if(gesture.direction[0]>0){
					swipeDirection="right";
				}else{
					swipeDirection="left";
				}
			
			}
		
		}
	
	}



});
*/
controller.on('gesture', function(gesture){
	var controllerOptions = {enableGestures:true};

	Leap.loop(controllerOptions,function(frame){

	if(frame.gestures.length>0){
		for(var i=0;i<frame.gestures.length;i++){
			var gesture =frame.gestures[i];
			if(gesture.type=="swipe"){
				var isHorizontal=Math.abs(gesture.direction[0])>Math.abs(gesture.direction[1]);
				if(gesture.direction[0]>0){
					swipeDirection="right";
					a=true;
				}else{
					swipeDirection="left";
					a=false;
				}
			
			}
			if(gesture.type=="keyTap"){
				//do something
				b=true;	
			}
			if(gesture.type=="circle"){
				var pointableID=gesture.pointableIds[0];
				var direction = frame.pointable(pointableID).direction;
				var dotProduct=Leap.vec3.dot(direction,gesture.normal);

				if(dotProduct>0){
					clockwise=true;
					c=true;
				}else{
					clockwise=false;
					c=false;
				}
			}
		
		}
	
	}



});
});

if(a==true){
	remote.send('KEY_POWERON');
}	
	/*
	remote.send(0xE0E0E0F1,function callback(err){
		if(err){
			console.log("err");
			//throw new Error(err);
		}else{
			console.log("command sent");
			//command has been transmitted
		}
	
	});
	
	//remote.isAlive(function(err){
	//	if(err){
	//		throw new Error('TV is offline');
	//	}
	
	//});
}

//	if (swipeDirection="right"){
	//if(circle.pointable().direction().angleTo(circle.normal())<=Leap::PI/2){
//		a = false;
//	}else a = true;
*/


controller.connect();

board = new five.Board();

board.on('ready', function() {
	//led = new five.Led(3);
	led = new five.Led(13);
	led1 = new five.Led(12);
	led2 = new five.Led(9);
	var ir = new five.IR.Reflect();
	//remote.send('KEY_UP');
	//IRsend irsend;
	this.loop(30, function() {
		if (a){ 
			//led.on(0xE0E0E01F,32);
			ir.on(0xE0E0E0F1,function(){
				console.log(0xE0E0E0F1);
			});
			led.on();
		}else{led.off();}
		if(b){
			led1.on();
		}
		if(c){
			led2.on();
		}else{led2.off();}
		
		//else led.sendSamsung(0x90,12);
	});
});

