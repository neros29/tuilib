### Character Object
The charecter object is used to hold the values and styles of each charecter that is to be displyed and return a valid ansi code for the style and charecter used. This object will includ:
- **charecter:** a single charecter to be displaed (eg. "h" or "5" or even "😍")
- **For Ground Color:** the text color using 24 bit color format or in outher words rgb (eg. `(255, 0, 0)` ) depending on the terrminl this may be downgraded to outher color formates
- **Background Color:** the background color of that charecter also rgb this will also if reqired be downgraded to outher color formats
- **Style:** this will depend on what styles are acutle possable but will use string codes (eg `"bold"` )
- There may be more 
### Surface Object
The surface object will be what widgets are created from. Think pygame surface a matrix of [[Rendering#Charecter Object]]s. These will be renderable and can be placed any ware on the screen or off the screen. This object will includ:
- **Surface** the main surface holding all the [[Rendering#Charecter Object]] 
- **Cordanits** in the main [[Rendering#Screen Object]] object of the 0,0 cordanate of the surface (eg to display the surface on the full screen the cordantes would be 0,0) this will be a intager array holding two values of x and y
- **Z Level** the layer in wich this screen responsable for deceiding if it's renderd on top or underneath outher surface objects
- 
### Screen Object
The screen object will represent the acutle terrminal window and thus will be what is rendered on to it. The screen will be created by going for each charecter in the screen matrix genrated by the terrminal size and cheaking every surfaces relative charecter and rendering the top most Z level charachter as that points charecter. This will allow for the ablity to have widgets that store information off screen to be easy to build. The screen object will includ:
- **Screen** The main screen matrix that will be rendered
- **Filler** The charecter object to be used in any place on the screen that no surface covers can be thouhg of as the background
- This object will also hold a pointer to every single surface that is to be rendered 
- **Size** the value of `ioctl(STDOUT_FILENO, TIOCGWINSZ, &((struct winsize){0,0,0,0}))`
- **Flip** function will be used to finaly display the screen on to the terrminal