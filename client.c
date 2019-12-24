#include <gtk/gtk.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h> 

#define PORT 8080 
#define WIDTH 8
#define HEIGHT 8


//0 = empty, 1 = black, 2 = white
int cells[8][8] = {
    {0, 1, 0, 1, 0, 1, 0, 1},
    {1, 0, 1, 0, 1, 0, 1, 0},
	{0, 1, 0, 1, 0, 1, 0, 1},
    {0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0},
    {2, 0, 2, 0, 2, 0, 2, 0},
	{0, 2, 0, 2, 0, 2, 0, 2},
    {2, 0, 2, 0, 2, 0, 2, 0}
};

struct GtkImage* squares[8][8];

//turn and piece;
int turn[];

//button clicked function
void callback( GtkWidget *widget, gpointer nr);

//initial location of piece. -1 = no piece selected
int loc = -1;

// Window of the application.
GtkWidget *window;

// Container for the contents of the application.
GtkWidget* contents;

// Contains the squares of the board and the pieces placed on these squares.
GtkGrid* board;



int sock = 0;
struct sockaddr_in serv_addr;

void setUpSocket(char* ip) {
    int valread;
    char *hello = "Hello from client"; 
    char buffer[1024] = {0}; 
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
        g_print("\n Socket creation error \n"); 
        return -1; 
    } 
   
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(PORT);
	
    // Convert IPv4 and IPv6 addresses from text to binary form 
    if(inet_pton(AF_INET, ip, &serv_addr.sin_addr)<=0)  
    { 
        g_print("\nInvalid address/ Address not supported \n"); 
        return -1; 
    } 
   
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        g_print("\nConnection Failed. Server not started? \n"); 
        return -1; 
    }
	
	recv(sock,turn,sizeof(turn),0);
	g_print("%d",turn[0]);
	int move[5];
	int len;
	while (1) {
		len = recv(sock,move,sizeof(turn),0);
		if (len > -1) {
			if (turn[1] == 1) {
				setCell_(move[0],move[1],0);
				setCell_(move[2][3],2);
				cells[move[0]][move[1]] = 0;
				cells[move[2]][move[3]] = 2;		
			}
			else {
				setCell_(move[0],move[1],0);
				setCell_(move[2][3],1);
				cells[move[0]][move[1]] = 0;
				cells[move[2]][move[3]] = 1;
			}
			if (move[4] == 1) {
				setCell_(move[0]+1,move[1]+1,0);
				cells[move[0]+1][move[1]+1] = 0;
			}		
		}	
	}
    //while ((valread = recv(sock,valread,sizeof(valread),0)) < 0) {
	//	
	//} 
    return 0; 

}


void setCell_(int row,int col,int piece) 
{	
	if (piece == 1) 
	{	
		gtk_widget_set_opacity (squares[row][col], 1);
		gtk_image_set_from_file(squares[row][col],"resources/images/bp.png");
		
	}
	
	else if (piece == 2) 
	{
		gtk_widget_set_opacity (squares[row][col], 1);
		gtk_image_set_from_file(squares[row][col],"resources/images/wp.png");
		
	}
	else if (piece == 3) 
	{
		gtk_widget_set_opacity (squares[row][col], 1);
		gtk_image_set_from_file(squares[row][col],"resources/images/bk.png");
		
	}
	else if (piece == 4) 
	{
		gtk_widget_set_opacity (squares[row][col], 1);
		gtk_image_set_from_file(squares[row][col],"resources/images/wk.png");
		
	}
	else
		gtk_widget_set_opacity (squares[row][col], 0);
	gtk_widget_show(squares[row][col]);
}


void callback( GtkWidget *widget, gpointer nr)
{	
	
	
	
	int num = GPOINTER_TO_INT(nr);
	if (loc == -1 && turn[0] == 1) {
		int col = num % 8;
		int row = num/8;
		
		if (cells[row][col] == turn[1]) {
			loc = num;
		}
	}
	
	else if (loc > -1 && loc != num) {
		int dest_col = num % 8;
		int dest_row = num/8;
		
		int initial_col = loc%8;
		int initial_row = loc / 8;
		
		if (cells[initial_row][initial_col] != 0) {
			//New Lines Below -DS
			
			int piece = cells[initial_row][initial_col];
			
			if (dest_row == 0 && piece == 2){
				piece = 4;
			}
			else if (dest_row == 7 && piece == 1){
				piece = 3;
			}
			
			int move[4] = {initial_row,initial_col,dest_row,dest_col};
			int response = send(sock , move , sizeof(move) , 0 );
			
			while (response < 0) {
				g_print("send error, trying again in 3 seconds");
				response = send(sock , move , sizeof(move) , 0 );
			}
			
			int update[5];
			recv(sock,update,sizeof(update),0);
			if (update[0] >-1) {
			//update initial and destination in GUI
			setCell_(update[2],update[3],piece);
			setCell_(update[0],update[1],0);
			
			//setCell_(dest_row,dest_col,piece,num);
			//setCell_(initial_row,initial_col,0,loc);
			
			//update initial and destination in game state
			cells[update[0]][update[1]] = 0;
			cells[update[2]][update[3]] = piece;
			
			//cells[initial_row][initial_col] = 0;
			//cells[dest_row][dest_col] = piece;
			
			if (abs( update[0] - update[1]) == 2 && abs( update[0] - update[1]) == 2 && update[4] == 1) {
				setCell_(update[2]-1,update[3]-1,0);
				cells[update[2]-1][update[3]-1] = 0;
			}
			
			turn[0] = 0;
			}
			
		}
		
		loc = -1;
	}
}

static void activate (GtkApplication* app, gpointer user_data)
{
	
	window = gtk_application_window_new (app);
	gtk_window_set_title (GTK_WINDOW (window), "Window");
	gtk_window_set_default_size (GTK_WINDOW (window), 480, 620);
	gtk_widget_show_all (window);
   
	contents = gtk_box_new(GTK_ORIENTATION_VERTICAL, 25);

	board = gtk_grid_new();

	GtkWidget* overlay_test = gtk_overlay_new();

	for (int i = 0; i < HEIGHT; ++i) {
		for (int j = 0; j < WIDTH; j++) {
			GtkWidget* img = gtk_image_new_from_file("resources/images/empty.png");
			gtk_widget_show(img);
			gtk_grid_attach(board, img, j, i, 1, 1);
		}
	}
	 
	int counter = 0;
	GtkWidget* button;
	for (int row = 0; row < WIDTH; ++row) {
		for (int col = 0; col < HEIGHT; ++col) {
			
			GtkWidget* img;
			
			// Color of the square is white.
			if ((row % 2 == 0 && col % 2 == 0) || (row % 2 != 0 && col % 2 != 0)) 
			{
				
				GtkWidget* overlay = gtk_overlay_new();
				gtk_widget_show(overlay);
				
				
				GtkWidget* square = gtk_image_new_from_file("resources/images/ws.png");
				gtk_widget_show(square);
				gtk_overlay_add_overlay(overlay, square);
				
				img = gtk_image_new_from_file("resources/images/bp.png");
				gtk_widget_set_opacity (img, 0);
				gtk_widget_show(img);
				gtk_overlay_add_overlay(overlay, img);
				
				button = gtk_button_new();
				gtk_widget_set_opacity (button, 0);
				g_signal_connect(button, "clicked", G_CALLBACK(callback), GINT_TO_POINTER(counter));
				gtk_widget_show(button);
				counter++;
				gtk_overlay_add_overlay(overlay, button);
				
				overlay_test = overlay;
				gtk_grid_attach(board, overlay, col, row, 1, 1);
				
			}
			
			// Color of the square is black. Piece also needs to be placed
			else 
			{
				
				GtkWidget* overlay = gtk_overlay_new();
				gtk_widget_show(overlay);

				GtkWidget* square = gtk_image_new_from_file("resources/images/bs.png");
				gtk_widget_show(square);
				gtk_overlay_add_overlay(overlay, square);
				
				// Add black piece to square.
				if (cells[row][col] == 1) {
					img = gtk_image_new_from_file("resources/images/bp.png");
					gtk_widget_show(img);
					gtk_overlay_add_overlay(overlay, img);
				}
				
				// Add white piece to square.
				else if (cells[row][col] == 2) {
					img = gtk_image_new_from_file("resources/images/wp.png");
					gtk_widget_show(img);
					gtk_overlay_add_overlay(overlay, img);
				}
				
				else {
					img = gtk_image_new();
					gtk_widget_set_opacity (img, 0);
					gtk_widget_show(img);
					gtk_overlay_add_overlay(overlay, img);
				}
				
				button = gtk_button_new();
				gtk_widget_set_opacity (button, 0);
				g_signal_connect(button, "clicked", G_CALLBACK(callback), GINT_TO_POINTER(counter));
				gtk_widget_show(button);
				counter++;
				gtk_overlay_add_overlay(overlay, button);
				overlay_test = overlay;

				gtk_grid_attach(board, overlay, col, row, 1, 1);
			}
			squares[row][col] = img;
			
		}
	}
	
	
	GtkWidget* test = gtk_grid_get_child_at(board, 0, 0);
	test = overlay_test;
	
	gtk_container_add(contents, board);

	/* This packs the button into the window (a gtk container). */
	gtk_container_add(GTK_CONTAINER(window), contents);

	gtk_widget_show(board);

	gtk_widget_show(contents);

	gtk_widget_show(window);
	
	//asking for IP
	GtkDialog *dialog;
	
	dialog = gtk_dialog_new_with_buttons ("IP",NULL,0,"OK",NULL,1);
	
	GtkEntryBuffer *buffer = gtk_entry_buffer_new (NULL,-1);
	GtkEntry* entry = gtk_entry_new_with_buffer (buffer);
	gtk_widget_show(entry);
	GtkWidget* vbox = gtk_dialog_get_content_area(dialog);
	gtk_container_add(GTK_CONTAINER(vbox),entry);
	
	int result = gtk_dialog_run(GTK_DIALOG(dialog));
	if (result = 1) {
		serv_addr.sin_addr.s_addr = inet_addr(gtk_entry_get_text (entry));
	}
	gtk_widget_destroy(dialog);
	
	
	
	
}

int
main (int    argc,
      char **argv)
{
  
  
  GtkApplication *app;
  int status;

  app = gtk_application_new ("org.gtk.checkers", G_APPLICATION_FLAGS_NONE);
  g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
  status = g_application_run (G_APPLICATION (app), argc, argv);
  g_object_unref (app);

  return status;
}
