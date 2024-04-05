/*
 *  timer.h
 */

/*
  this is an abstract timer.
  an assetVar ( or anything esle (can ask for a timer.
     = send a message to the timer with a timeout value a timeout code, a call back function
     so if a recuring timer is needed  the asset setvar will always also call a timer.

       assetVar->timersetval(15)
       timer_chan   <<  "assetvar->timesetval 16 now + 1 sec"
// this is used for heartbeats a maxval param will be used  to reset after say 255 
//  and another param will used to set the interval

// (assetVar->setparam("maxtick", 255))
// (assetVar->setparam("ticktime", 1.0))


// another example is a toggle in this case the timer callback will set the value back to oldval
//      assetVar->togglesetval(1)
//      timer_chan   <<  "assetvar->togglesetval 0 now + 1 sec"

// and another is setting a timeout on a set or get message
//      timer_chan <<  /replyto/@@@@@@ assetvar->fimstimeout(@@@@@@) 5.0
// this message will call the fimstimeout function with  for the assetvar with the code @@@@@@ after 5 seconds.
// if a fims message is sent to /replyto/@@@@@@ the timeout will not be delivered.
// todo find a neat way to do this. 


