#include <gtk/gtk.h>
#include <stdio.h> 
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
	
    send(sock , hello , strlen(hello) , 0 );
	recv(sock,valread,sizeof(valread),0);
	g_print(valread);
    //while ((valread = recv(sock,valread,sizeof(valread),0)) < 0) {
	//	
	//} 
    return 0; 

}


void setCell_(int row,int col,int piece,int num) 
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
	else
		gtk_widget_set_opacity (squares[row][col], 0);
	gtk_widget_show(squares[row][col]);
}


void callback( GtkWidget *widget, gpointer nr)
{	
	/*	GtkWidget *dialog;
	dialog = gtk_message_dialog_new(GTK_WINDOW(window),
            GTK_DIALOG_DESTROY_WITH_PARENT,
            GTK_MESSAGE_INFO,
            GTK_BUTTONS_OK,
            "Download Completed");
	gtk_window_set_title(GTK_WINDOW(dialog), "Information");
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	*/
	
	
	int num = GPOINTER_TO_INT(nr);
	g_print ("loc: %d\n",loc);
	if (loc == -1) {
		int col = num % 8;
		int row = num/8;
		
		
	
		g_print ("num: %d\nrow: %d\ncol: %d\ncell value: %d\n",num,row,col);
		
		if (cells[row][col] != 0) {
			loc = num;
			g_print ("inside if\n");
		}
	}
	
	else if (loc > -1 && loc != num) {
		int dest_col = num % 8;
		int dest_row = num/8;
		
		int initial_col = loc%8;
		int initial_row = loc / 8;
		
		if (cells[initial_row][initial_col] != 0) {
			int piece = cells[initial_row][initial_col];
			g_print("dest\n");
			//update destination cell
			setCell_(dest_row,dest_col,piece,num);
			g_print("init\n");
			//update initial cell
			setCell_(initial_row,initial_col,0,loc);
			
			cells[initial_row][initial_col] = 0;
			cells[dest_row][dest_col] = piece;
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
				g_print("yolo\n\n\n\n");
				
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
