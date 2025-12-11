Cache ansii sequences if this is a slow point
defnitly only acutly print to screen the charecters that changed and acutly exist
Use rich an texutal as guids when it comes to rendering in the terrminal
Get terrminal size using `ioctl(STDOUT_FILENO, TIOCGWINSZ, &((struct winsize){0,0,0,0}))`
- using constructer destructer combo for raw mode and normal mode
- use a vector of dirty flags that is the same size as the acutle screen 
- 