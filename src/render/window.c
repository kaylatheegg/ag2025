#include "desertbus.h"
#include "colours.h"

da(entry) palette;

term_info render_info = {0};

struct termios term_settings;

window* new_window(int width, int height) {
    if (width <= 0) width = render_info.width;
    if (height <= 0) height = render_info.height;

    window* cur_win = dbmalloc(sizeof(*cur_win));
    cur_win->window_data = dbmalloc(sizeof(*cur_win->window_data) * height);
    for (int i = 0; i < height; i++) {
        cur_win->window_data[i] = dbmalloc(sizeof(**cur_win->window_data) * width * 2 + 1);
    }
    
    cur_win->width = width;
    cur_win->height = height;
    return cur_win;
}

void destroy_window(window* win) {
    for (int i = 0; i < win->height; i++) {
        dbfree(win->window_data[i]);
    }
    dbfree(win->window_data);
    dbfree(win);
}

void add_colour(char* string, char* colour) {
    if (palette.at == NULL) {
        da_init(&palette, 1);
        add_colour("DEFAULT", "\033[38;5;201;48;5;201m");
    }
    entry new_entry = {string, colour};
    da_append(&palette, new_entry);
}

void render_win(window* win) {
    if (win == NULL) return;
    if (palette.at == NULL) return;

    fwrite("\x1b[H", 1, 3, stdout); //reposition the cursor

    fwrite("\x1b[0m", 1, 4, stdout);

    for (int i = 0; i < win->height_offset; i++) {
        fwrite("\x1b[1E", 1, 4, stdout);
    }

    for (int i = 0; i < win->height && (i + win->height_offset) < render_info.height; i++) {
        char char_buf[1024];
        int chars = 0;
        for (int j = 0; j < win->width_offset; j++) fwrite("\033[1Cm", 1, 4, stdout);

        for (int j = 0; j < win->width && (j + win->width_offset) < render_info.width; j++) {  
            chars = 0;
            if (win->window_data[i][2*j + 1] == '\0') chars = sprintf(char_buf, "\033[1C");
            else chars = sprintf(char_buf, "%s%c", palette.at[win->window_data[i][2*j]].colour, win->window_data[i][2*j + 1]);
            fwrite(char_buf, 1, chars, stdout);
        }
        if (i != (win->height - 1)) {
            fwrite("\x1b[0m", 1, 4, stdout);
            fwrite("\x1b[1E", 1, 4, stdout); 
        }
    }
}

void get_render_dimensions() {
    char* term = getenv("TERM");
    if (term == NULL) crash("TERM environment variable not set\n");

    char* cterm_path = ctermid(NULL);
    if (cterm_path == NULL) crash("ctermid failed! err: %s\n", strerror(errno));

    int tty_fd = open(cterm_path, O_RDWR);
    if (tty_fd == -1) crash("open(\"%s\") failed: %s\n", cterm_path, strerror(errno));
    
    int err;
    if (setupterm(term, tty_fd, &err) != 0) {
        switch (err) {
            case -1: crash("terminfo database not found\n");
            case 0:  crash("TERM=%s not found in database\n", term);
            case 1:  crash("terminal is hardcopy\n");
        }
    }
    //we're gonna IGNORE any errors and hope for good 

    render_info.width = tigetnum("cols");  // number of (co)lumns
    if (render_info.width == -1) crash("tigetnum(\"cols\") failed\n");
    render_info.height = tigetnum("lines");
    if (render_info.height == -1) crash("tigetnum(\"lines\") failed\n");
    close(tty_fd);
}

void init_render() {
    get_render_dimensions();

    //set terminal into raw mode
    //get old terminal settings so we can restore them on exit
    tcgetattr(STDIN_FILENO, &term_settings);

    //get some new ones
    struct termios new_info;
    tcgetattr(STDIN_FILENO, &new_info);
    //disables ^S and ^Q
    //disables ^M
    //disables output processing
    //disables echo
    //disables canonical, so we can get byte by byte info
    //disables ^V
    //disables ^C and ^Z
    //BRKINT, INPCK, ISTRIP and CS8 are here for "true" rawmode, just for all termemu support
    new_info.c_iflag &= ~(BRKINT| ICRNL |INPCK|ISTRIP| IXON);
    new_info.c_oflag &= ~(OPOST);
    new_info.c_cflag |= (CS8);
    new_info.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &new_info);

    /* Disable cursor */
    write(STDOUT_FILENO, "\x1b[?25l", 6);
    
    /* Use alternate screen buffer */
    write(STDOUT_FILENO, "\x1b[?1046h", 8);
    write(STDOUT_FILENO, "\x1b[?1049h", 8);

    return;
}

void exit_render() {
    //fix the terminal
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &term_settings);

    write(STDOUT_FILENO, "\033[0m", 4);
    //clear and fix cursor
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);
    write(STDOUT_FILENO, "\x1b[?25h", 6);
    write(STDOUT_FILENO, "\x1b[?1049l", 8);
}