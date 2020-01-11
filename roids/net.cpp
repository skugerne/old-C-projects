#include "main.hpp"



void initNet(){
  // getting something that passes for our IP address + port
  if( SDLNet_ResolveHost(&_thisIP,NULL,9999) == -1 ){
    printf("ERROR - SDLNet_ResolveHost: %s\n",SDLNet_GetError());
    exit(1);
  }
  
  // open the server socket
  // this will only get used if the player hosts a game
  _hostSocket=SDLNet_TCP_Open(&_thisIP);
  if(!_hostSocket){
    printf("ERROR - SDLNet_TCP_Open: %s\n",SDLNet_GetError());
    exit(1);
  }
  
  // a socketSet allows non-blocking status checks on groups of sockets
  _sockSet=SDLNet_AllocSocketSet(1);
  if(!_sockSet){
    printf("ERROR - SDLNet_AllocSocketSet: %s\n", SDLNet_GetError());
    exit(1);
  }
}



void networkStartupHost(){

}



void networkStartupJoin(){
  
}



void networkShutdown(){

}



// this looks for a connection but is not gaurenteed to do anything
bool hostListen(){
  // try to accept a connection
  TCPsocket tempSocket;
  tempSocket=SDLNet_TCP_Accept(_hostSocket);
  if(tempSocket){
    
    // nuke whatever networking is running now
    networkShutdown();
    
    // store our new socket
    _otherSocket = tempSocket;
    
    IPaddress *tempIP = SDLNet_TCP_GetPeerAddress(_otherSocket);
    if(!tempIP){
      _otherIP = *tempIP;
      printf("ERROR - SDLNet_TCP_GetPeerAddress: %s\n",SDLNet_GetError());
      exit(1);
    }
    
    return true;
  }
  
  return false;
}



bool clientConnect(char *ipString){
  // setting up to connect to server at default port
  if( SDLNet_ResolveHost(&_otherIP,ipString,9999) == -1 ){
    printf("ERROR - SDLNet_ResolveHost: %s\n",SDLNet_GetError());
    exit(1);
  }
  /*
  // open the connection
  _otherSocket SDLNet_TCP_Open(&_otherIP);
  if(!_otherSocket){
    printf("ERROR - SDLNet_TCP_Open: %s\n",SDLNet_GetError());
    exit(1);
  }
  
  int SDLNet_TCP_AddSocket(SDLNet_SocketSet set, TCPsocket sock)
  */
  
  return true;
}



bool doGreetings(){
  if(_netStatus == GAME_HOST){
    char mesg[9] = "Roids v5";
    if( SDLNet_TCP_Send(_otherSocket,(void*)&mesg[0],9) < 9 ){
      printf("ERROR - SDLNet_TCP_Send: %s\n",SDLNet_GetError());
      return false;
    }else{
      #ifdef DEBUG_NET
      fprintf(stderr,"Greetings sent.\n");
      #endif
      return true;
    }
  }else{
    if( SDLNet_CheckSockets(_sockSet, 10) == -1 ){
      printf("ERROR - SDLNet_CheckSockets: %s\n",SDLNet_GetError());
      return false;
    }else if( SDLNet_SocketReady(_otherSocket) ){
      char data[9];
      int got = SDLNet_TCP_Recv(_otherSocket, (void*)&data[0], 9);
      if(got>0){
        if(data == "Roids v5"){
          #ifdef DEBUG_NET
          fprintf(stderr,"Greetings recieved.\n");
          #endif
          return true;
        }
      }
    }
  }
  
  return false;
}



bool doObjectTransfer(objecttype *oPtr){
  return true;
}



bool doEndGreetings(){
  return true;
}



// this function returns 2 to the power given
Uint pow2(Uint power){
  return (1 << power);
}



// this function is supposed to make it easy to interpret packets by ...
// ... getting the integer value of any range of bits (up to 16) ...
// ... from the given char string, which would probably be packet data ...
// ... for example
// start and stop are inclusive
// bits start numbering at 0
Uint getIntValFromBitRange(Uchar *data, Uint start, Uint stop){
  Uint range = stop - start + 1;
  
  // the value this function seeks
  Uint value=0;
  
  Uint tempvalue;
  Uint power;
  Uint charIndex;
  Uint startBitIndexWithinChar;
  Uint numBitsForThisChar;
  
  //********************************************************************
  // determine first char, get value from it
  charIndex = start / 8;                    // 0 through (charLength - 1)
  startBitIndexWithinChar = start % 8;      // 0 through 7
  numBitsForThisChar = 8 - startBitIndexWithinChar;
  
  // does the range fit within this first character ?
  if(range < numBitsForThisChar){
    // case where the entire range falls within this charIndex
    // ok, first we figure out power, which is this case is used to ...
    // ... remove the spare bits to the right of our range with / ...
    // ... then we remove the spare bits to the left with % ...
    // ... range is set to zero to show we're done
    range = 0;
    power = pow2(7 - (stop % 8));
    tempvalue = data[charIndex] % pow2(8 - startBitIndexWithinChar);
    tempvalue /= power;
    value = tempvalue;
  }else{
    // anyway, lets continue with the non-special case...
    // first we figure out power, which is this case is used to ...
    // ... account for still un-added bits in char's to the right ...
    // ... then we remove the spare bits to the left with % ...
    // ... range made smaller to show we've made progress and ...
    // ... start is moved along to index 0 of next character
    range -= numBitsForThisChar;
    start += numBitsForThisChar;
    power = pow2(range);
    tempvalue = data[charIndex] % pow2(8 - startBitIndexWithinChar);
    tempvalue *= power;
    value = tempvalue;
  }
  
  // if we have handled the whole thing with 1st char, otherwise...
  if(range == 0) return value;
  
  // from the previous stage, it is given that start is now at the start ...
  // ... of character charIndex + 1
  
  ++charIndex;
  if(range > 7){
    // we have this whole character to add on
    
    numBitsForThisChar = 8;
    range -= numBitsForThisChar;
    start += numBitsForThisChar;
    power = pow2(range);
    tempvalue = data[charIndex];
    tempvalue *= power;
    value += tempvalue;
  }else{
    // add in the part of this char that we do have
    
    range = 0;
    power = pow2(7 - (stop % 8));
    tempvalue = data[charIndex];
    tempvalue /= power;
    value += tempvalue;
  }
  
  // if we have handled the whole thing with 1st + 2nd chars, otherwise...
  if(range == 0) return value;
  
  // and having got this far, there is only one possible case given ...
  // ... the restriction that the range is no more than 16 bits ...
  // ... and this case is: starts and char's index of 0, does not ...
  // ... reach the end of the char
  
  ++charIndex;
  power = pow2(7 - (stop % 8));
  tempvalue = data[charIndex];
  tempvalue /= power;
  value += tempvalue;
  
  return value;
}



// this function allows a bit range of the given char string to be set ...
// ... to the given positive int value.... this function is designed to ...
// ... simplify networking
// start and stop are inlclusive
// bits start numbering at 0
Uchar* setBitRangeToIntVal(Uint value, Uchar *data, Uint start, Uint stop){
  Uint range = stop - start + 1;
  
  Uint rightCharIndex;
  Uint rightBitIndex;
  Uint bitRange;
  Uint valForBitRange;
  
  // pow2(bitRange) is the largest value that can be stored in those bits
  
  rightCharIndex = stop / 8;
  rightBitIndex = stop % 8;
  bitRange = rightBitIndex + 1;
  
  valForBitRange = value % pow2(bitRange);
  valForBitRange *= pow2(7 - rightBitIndex);
  data[rightCharIndex] += valForBitRange;
  
  if(bitRange > range){
    return data;
  }else{
    value = value / pow2(bitRange);
    stop -= (rightBitIndex + 1);
    range = stop - start + 1;
  }
  
  if(range == 0) return data;
  
  --rightCharIndex;
  // if we made it this far, we know that our rightBitIndex is 7
  
  if(range > 7){
    // we now know that we have those whole character, so left most ...
    // ... bit is zero and rightmost is 7
    
    range -= 8;
    stop -= 8;
    valForBitRange = value % 256;
    data[rightCharIndex] += valForBitRange;
    value = value / 256;
  }else{
    // the value of "value" should now equal what we want without any ...
    // ... changes because of the line "value = value / pow2(bitRange);" ...
    // ... above where the first char was resolved
    
    range = 0;
    data[rightCharIndex] += value;
  }
  
  if(range == 0) return data;
  
  // now we still know right most bit is at spot 7, but we also know ...
  // ... that the leftmost is not at spot 0
  
  --rightCharIndex;
  data[rightCharIndex] += value;
  
  return data;
}



void spellOutString(Uint len, char *data){
  Uchar* undata = (Uchar*)data;
  
  for(Uint i=0;i<len;++i){
    fprintf(stderr,"[%u]",undata[i]);
  }
  fprintf(stderr,"\n");
}
