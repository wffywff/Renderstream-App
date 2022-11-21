# RenderStream App
### Product Engineering Code Test Project

RenderStream App is an application that utilizes [RenderStream API](https://github.com/disguise-one/RenderStream) offered by Disguise.

## Implemented:(as for 21/11/2022)
* default render a spinning cube and send over renderstream using dx11
* each render corresponds with a window that shows the texture being sent over renderstream

## Design Goals:
This application can simulate failure conditions easily to faciliate with troubleshooting of Renderstream, for example:
* one render instance taking longer than the rest 
* one render instance has crashed 
* display fps of the render instance
* display network information

This application is not designed for production, therefore error handling aims to illustrates failure to user quickly.
