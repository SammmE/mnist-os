// We will write directly to Video Memory to print "X"
void kmain() {
    // 0xB8000 is the address of the text screen in video memory
    char* video_memory = (char*) 0xb8000;
    
    // The first byte is the character
    *video_memory = 'X'; 
    
    // The second byte is the color (White on Black)
    *(video_memory + 1) = 0x0f; 
}
