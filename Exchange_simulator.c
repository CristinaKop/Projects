#include "spx_exchange.h"
#include <poll.h>

volatile int pipe_signal = FALSE;
volatile pid_t trader_sig_id = 0;
volatile pid_t disconnect_child = 0;
volatile pid_t old_disconnect = 0;

/* Function: set_up_trader
 * ----------------------------
 *   Opens the pipes for the trader and sets up the trader_struct.
 *
 *   trader: the executable file name of the trader
 *   trader_id: the trader's id
 * 	 exchange_trader: the struct to populate with trader information
 *   size: the size of thej product array
 *   product_array: the array that stores the products as strings
 */
void set_up_trader(char *trader, int trader_id, struct trader_struct *exchange_trader, int size, char **product_array)
{
	int trader_length = snprintf(NULL, 0, "%d", trader_id);
	char *exchange_t_pipe = malloc(sizeof(char) * (strlen(FIFO_EXCHANGE) - 1 + trader_length));
	char *trader_e_pipe = malloc(sizeof(char) * (strlen(FIFO_TRADER) - 1 + trader_length));
	sprintf(exchange_t_pipe, FIFO_EXCHANGE, trader_id);
	sprintf(trader_e_pipe, FIFO_TRADER, trader_id);
	mkfifo(exchange_t_pipe, MKFIFO_PERMISSION);
	mkfifo(trader_e_pipe, MKFIFO_PERMISSION);
	int pid = fork();

	if (pid == 0)
	{
		printf("%s Starting trader %d (%s)\n", LOG_PREFIX, trader_id, trader);
		execl(trader, trader, exchange_t_pipe + strlen(FIFO_EXCHANGE) - 2, NULL);
	}
	else
	{
		int exchange_fd = open(exchange_t_pipe, O_WRONLY);
		FILE *exchange_t_fp = fdopen(exchange_fd, "w");
		printf("%s Connected to %s\n", LOG_PREFIX, exchange_t_pipe);

		exchange_trader->trader_fd = open(trader_e_pipe, O_RDONLY);
		FILE *trader_e_fp = fdopen(exchange_trader->trader_fd, "r");
		printf("%s Connected to %s\n", LOG_PREFIX, trader_e_pipe);

		exchange_trader->trader_id = trader_id;
		exchange_trader->pipe_exchange_t = exchange_t_pipe;
		exchange_trader->pipe_trader_e = trader_e_pipe;
		exchange_trader->fp_exchange_t = exchange_t_fp;
		exchange_trader->fp_trader_e = trader_e_fp;
		exchange_trader->pid_child = pid;
		exchange_trader->positions = malloc(sizeof(struct trader_positions) * size);
		exchange_trader->alive = TRUE;
		exchange_trader->order_valid = 0;
		for (size_t i = 0; i < size; i++)
		{

			exchange_trader->positions[i].product = product_array[i];
			exchange_trader->positions[i].quantity = 0;
			exchange_trader->positions[i].price = 0;
		}
	}
}

/* Function: get_products_size
 * ----------------------------
 *   Reads the first item from the file of how many products there are.
 *
 *   file: the name of the file path
 * 	 returns: numbers of products in the file
 */
int get_products_size(char *file)
{
	FILE *ptr = fopen(file, "r");
	char string[PRODUCT_SIZE];
	fgets(string, PRODUCT_SIZE, ptr);
	return atoi(string);
}

/* Function: load_products_file
 * ----------------------------
 *   Loads the products of the file into a product array.
 *
 *   file_name: the name of the file path
 * 	 returns: an array of pointers to strings being the product names
 */
char **load_products_file(char *file_name)
{
	FILE *ptr = fopen(file_name, "r");
	char string[PRODUCT_SIZE];
	fgets(string, PRODUCT_SIZE, ptr);
	char **product_array = (char **)malloc(sizeof(char *) * atoi(string));
	int i = 0;
	while (i < atoi(string))
	{
		product_array[i] = (char *)malloc(sizeof(char) * PRODUCT_SIZE);
		fgets(product_array[i], PRODUCT_SIZE, ptr);
		product_array[i][strlen(product_array[i]) - 1] = '\0';
		i++;
	}
	fclose(ptr);
	return product_array;
}

/* Function: get_id_pid
 * ----------------------------
 *   Get the trader ID of a trader given their PID.
 *
 *   disconnect_child: the PID to match with the trader
 *   exchange_traders: linked list of trader_struct(s)
 *   size: number of products
 * 	 returns: the trader ID that belongs to a PID, FALSE if no match.
 */
int get_id_pid(pid_t disconnect_child, struct trader_struct *exchange_traders, int size)
{
	for (size_t i = 0; i < size; i++)
	{
		if (exchange_traders[i].pid_child == disconnect_child)
		{
			return exchange_traders[i].trader_id;
		}
	}
	return FALSE;
}

/* Function: te_sig
 * ----------------------------
 *   The signal handeler for trader writing to the exchange.
 *
 *   signo: signal number corresponding to the signal that needs to be handled
 *   sinfo: pointer to the signal handler function
 *   context: void pointer
 */
void te_sig(int signo, siginfo_t *sinfo, void *context)
{
	pipe_signal = TRUE;
	trader_sig_id = sinfo->si_pid;
}

/* Function: child_sig
 * ----------------------------
 *   The signal handeler for exchange writing to the trader.
 *
 *   signo: signal number corresponding to the signal that needs to be handled
 *   sinfo: pointer to the signal handler function
 *   context: void pointer
 */
void child_sig(int signo, siginfo_t *sinfo, void *context)
{
	disconnect_child = sinfo->si_pid;
}

/* Function: get_trader_e_fp
 * ----------------------------
 *   Get the file pointer to the trader to exchange pipe who sent a signal to the exchange.
 *
 *   pid: the pid to match
 *   exchange_traders: linked list of trader_struct(s)
 *   num_traders: number of traders in the exchange
 *   sent_id: pointer to an integer of the trader id that sent the signal
 * 	 returns: the file pointer to the trader to exchange pipe for the trader who sent a signal to the exchange.
 */
FILE *get_trader_e_fp(int pid, struct trader_struct *exchange_traders, int num_traders, int *sent_id)
{
	for (size_t i = 0; i < num_traders; i++)
	{
		if (exchange_traders[i].pid_child == pid)
		{
			*sent_id = exchange_traders[i].trader_id;
			return exchange_traders[i].fp_trader_e;
		}
	}
	return NULL;
}

/* Function: get_exchange_t_fp
 * ----------------------------
 *   Get the file pointer to the exchange to trader pipe for the trader who sent a signal to the exchange.
 *
 *   pid: the pid to match
 *   exchange_traders: linkes list of trader_struct(s)
 *   num_traders: number of traders in the exchange
 * 	 returns: the file pointer to the exchange to trader pipe for the trader who sent a signal to the exchange.
 */
FILE *get_exchange_t_fp(int pid, struct trader_struct *exchange_traders, int num_traders)
{
	for (size_t i = 0; i < num_traders; i++)
	{
		if (exchange_traders[i].pid_child == pid)
		{
			return exchange_traders[i].fp_exchange_t;
		}
	}
	return NULL;
}

/* Function: varstin
 * ----------------------------
 *   Reallocate a buffer if more input is required.
 *   Citation: Richard McKenzie: Week 9 Question 4, from atm.c https://edstem.org/au/courses/7900/workspaces/pqh57Fru0l6V2qlVV9zmrK8mVMVx4MXi
 *
 *   fp: file pointer of where to get the input for the buffer from
 * 	 returns: a buffer of characters taken from the file
 */
char *varstin(FILE *fp)
{
	int size = 1;
	int rs = BUFFSIZE;
	char *mstrout = malloc(rs);
	char gc;
	while (((gc = getc(fp)) != ';'))
	{
		if (gc == EOF)
		{
			free(mstrout);
			return NULL;
		}
		mstrout[size - 1] = gc;
		size++;
		if (size == rs)
		{
			rs += BUFFSIZE;
			mstrout = realloc(mstrout, rs);
		}
	}
	mstrout[size - 1] = '\0';
	return mstrout;
}

/* Function: check_product
 * ----------------------------
 *   Checks whether a product is in the product array
 *
 *   product: the product to check
 *   product_array: the product array
 *   size: the size of the product array
 * 	 returns: TRUE (1) if the product is in the array, and FALSE (0) otherwise
 */
int check_product(char *product, char **product_array, int size)
{
	for (size_t i = 0; i < size; i++)
	{
		if (strcmp(product, product_array[i]) == 0)
		{
			return TRUE;
		}
	}
	return FALSE;
}

/* Function: market_open
 * ----------------------------
 *   Loops over traders and writes market open, then loops over traders and sends signals
 *
 *   number_traders: the number of traders
 *   exchange_traders: the linked list of trader_struct(s)
 */
void market_open(int number_traders, struct trader_struct *exchange_traders)
{
	// Loop over traders and write market open
	for (size_t i = 0; i < number_traders; i++)
	{
		fprintf(exchange_traders[i].fp_exchange_t, "MARKET OPEN;");
		fflush(exchange_traders[i].fp_exchange_t);
	}
	// Loop over traders and send signals
	for (size_t i = 0; i < number_traders; i++)
	{
		kill(exchange_traders[i].pid_child, SIGUSR1);
	}
}

/* Function: print_trading
 * ----------------------------
 *   Prints the items in the product array
 *
 *   size: the number of products
 */
void print_trading(char **product_array, int size)
{
	printf("%s Trading %d products:", LOG_PREFIX, size);
	for (size_t i = 0; i < size; i++)
	{
		printf(" %s", product_array[i]);
	}
	printf("\n");
}

/* Function: initalise_traders
 * ----------------------------
 *   Loops over the traders and calls set_up_trader.
 *
 *   argv: the trader binaries from the command line
 *   exchange_traders: linked list of trader_struct(s)
 *   size: the number of products
 *   product_array: the array that stores the products as strings
 */
void initalise_traders(char **argv, int argc, struct trader_struct *exchange_traders, int size, char **product_array)
{
	int trader_id = 0;
	for (size_t i = 2; i < argc; i++)
	{
		set_up_trader(argv[i], trader_id, &(exchange_traders[i - 2]), size, product_array);
		trader_id++;
	}
}

/* Function: get_trader_id
 * ----------------------------
 *   Gets a trader from its trader id.
 *
 *   sent_id: the trader the sent a signal to the exchange
 *   exchange_traders: linked list of trader_struct(s)
 *   size: the number of products
 *   returns: the trader_struct of the trader with the id
 */
struct trader_struct *get_trader_id(int sent_id, struct trader_struct *exchange_traders, int size)
{

	if (sent_id >= size)
	{
		return NULL;
	}
	else
	{
		return exchange_traders + sent_id;
	}
}

/* Function: send_invalid
 * 	----------------------------
 *   Sends invalid to the trader.
 *
 *   pipe: the exchanage to trader pipe of the trader
 *   child: the PID of the trader
 */
void send_invalid(FILE *pipe, int child)
{
	fprintf(pipe, "INVALID;");
	fflush(pipe);
	kill(child, SIGUSR1);
}

/* Function: send_cancel
 * 	----------------------------
 *   Sends cancel to the trader.
 *
 *   pipe: the exchanage to trader pipe of the trader
 *   child: the PID of the trader
 *   order_id: the order id of the cancelled order
 */
void send_cancel(FILE *pipe, int child, int order_id)
{
	fprintf(pipe, "CANCELLED %d;", order_id);
	fflush(pipe);
	kill(child, SIGUSR1);
}

/* Function: send_fill
 * 	----------------------------
 *   Sends a fill order to the trader.
 *
 *   pipe: the exchanage to trader pipe of the trader
 *   child: the PID of the trader
 *   order_id: the order id of the cancelled order
 *   quantity: quantity of the filled order
 */
void send_fill(FILE *pipe, int child, int order_id, long int quantity)
{
	fprintf(pipe, "FILL %d %ld;", order_id, quantity);
	fflush(pipe);
	kill(child, SIGUSR1);
}

/* Function: make_current_order
 * 	----------------------------
 *   Create an 'order' by populating an order_type struct.
 *
 *   size: the number of products
 *   number_traders: the number of traders
 *   product_array: the array that stores the products as strings
 *   buff: the array which stores the order characters to be processed
 *   sent_id: the id of the trader that sent the order
 *   exchange_traders: linked list of trader_struct(s)
 */
struct order_type *make_current_order(int size, int number_traders, char **product_array, char *buff, int sent_id, struct trader_struct *exchange_traders)
{

	struct order_type *current_order = malloc(sizeof(struct order_type));
	current_order->trader = get_trader_id(sent_id, exchange_traders, number_traders);
	current_order->level = 1;
	current_order->prev = NULL;
	current_order->next = NULL;
	current_order->product = malloc(sizeof(char) * PRODUCT_SIZE);
	int line_num = 0;
	while (line_num < LINE_ITEMS)
	{
		char *line = strsep(&buff, " ");
		if (line == NULL)
		{
			send_invalid(current_order->trader->fp_exchange_t, current_order->trader->pid_child);
			free(current_order->product);
			free(current_order);
			return NULL;
		}
		// Command
		if (line_num == 0)
		{
			int order_type_int = 0;
			if (strcmp(line, "BUY") == 0)
			{
				order_type_int = BUY;
			}
			else if (strcmp(line, "SELL") == 0)
			{
				order_type_int = SELL;
			}

			current_order->type = order_type_int;
		}
		// Order id
		if (line_num == 1)
		{
			int order_id = atoi(line);
			if (current_order->trader->order_valid == order_id || order_id == UPPER_BOUND)
			{
				current_order->order_id = order_id;
			}
			else
			{
				send_invalid(current_order->trader->fp_exchange_t, current_order->trader->pid_child);
				free(current_order->product);
				free(current_order);
				return NULL;
			}
		}
		// Product
		else if (line_num == 2)
		{
			int product_exists = check_product(line, product_array, size);
			if (product_exists == FALSE)
			{
				send_invalid(current_order->trader->fp_exchange_t, current_order->trader->pid_child);
				free(current_order->product);
				free(current_order);
				return NULL;
			}
			else
			{

				strcpy(current_order->product, line);
			}
		}
		// Quantity
		else if (line_num == 3)
		{
			long int quantity = atoi(line);
			if (quantity > 0 && quantity < UPPER_BOUND)
			{
				current_order->quantity = atoi(line);
			}
			else
			{
				send_invalid(current_order->trader->fp_exchange_t, current_order->trader->pid_child);
				free(current_order->product);
				free(current_order);
				return NULL;
			}
		}
		// Price
		else if (line_num == 4)
		{
			char *strdod_ptr;
			long int price = strtod(line, &strdod_ptr);
			if (price > 0 && price < UPPER_BOUND)
			{
				current_order->price = price;
			}
			else
			{
				send_invalid(current_order->trader->fp_exchange_t, current_order->trader->pid_child);
				free(current_order->product);
				free(current_order);
				return NULL;
			}
		}
		line_num++;
	}

	char *line = strsep(&buff, " ");

	if (line != NULL)
	{

		send_invalid(current_order->trader->fp_exchange_t, current_order->trader->pid_child);
		free(current_order->product);
		free(current_order);
		return NULL;
	}

	current_order->trader->order_valid++;

	return current_order;
}

/* Function: free_traders
 * 	----------------------------
 *   Frees the memory of the trader_struct(s).
 *
 *   number_traders: the number of traders
 *   exchange_traders: linked list of trader_struct(s)
 */
void free_traders(int number_traders, struct trader_struct *exchange_traders)
{
	for (size_t i = 0; i < number_traders; i++)
	{
		fclose(exchange_traders[i].fp_exchange_t);
		fclose(exchange_traders[i].fp_trader_e);
		remove(exchange_traders[i].pipe_exchange_t);
		remove(exchange_traders[i].pipe_trader_e);
		free(exchange_traders[i].pipe_exchange_t);
		free(exchange_traders[i].pipe_trader_e);
		free(exchange_traders[i].positions);
	}
	free(exchange_traders);
}

/* Function: free_order_book
 * 	----------------------------
 *   Frees the memory of the order book.
 *
 *   order_book: array of orders
 *   size: the number of products
 */
void free_order_book(struct product_info *order_book, int size)
{
	for (size_t i = 0; i < size; i++)
	{
		struct order_type *orders = order_book[i].first_order;
		struct order_type *order_before;
		while (orders != NULL)
		{
			order_before = orders;
			orders = orders->next;
			free(order_before->product);
			free(order_before);
		}
	}
	free(order_book);
}

/* Function: free_product_array
 * 	----------------------------
 *   Frees the memory of the product array.
 *
 *   size: the number of products
 *   product_array: the array that stores the products as strings
 */
void free_product_array(int size, char **product_array)
{
	for (size_t i = 0; i < size; i++)
	{
		free(product_array[i]);
	}
	free(product_array);
}

/* Function: update_product_info
 * 	----------------------------
 *   Updates the product info buy and sell numbers for a product orderbook.
 *
 *   type: update buy or sell quantity
 *   product_node: the product_info struct to update
 */
void update_product_info(int type, struct product_info *product_node)
{
	if (type == BUY)
	{
		(product_node->buy)++;
	}
	else if (type == SELL)
	{
		(product_node->sell)++;
	}
}

/* Function: get_trader_positions
 * 	----------------------------
 *   Gets the trader's position for a product.
 *
 *   match_array: a trader_positions object which we compare to the product
 *   size: the number of products
 *   product: the product to match against
 */
struct trader_positions *get_trader_positions(struct trader_positions *match_array, int size, char *product)
{

	size_t i = 0;
	for (i = 0; i < size; i++)
	{
		if (strcmp(match_array[i].product, product) == 0)
		{
			return &(match_array[i]);
		}
	}
	return &(match_array[i]);
}

/* Function: custom_round
 * 	----------------------------
 *   Rounds the exchange fee to the nearest dollar.
 *
 *   to_round: the double to round
 *   returns: the rounded long int
 */
long int custom_round(double to_round)
{
	long int semi_round = (long int)to_round;
	if (to_round - semi_round >= 0.5)
	{
		return semi_round + 1;
	}

	return semi_round;
}

/* Function: remove_match_node
 * 	----------------------------
 *   Removes an order from the order book.
 *
 *   match_node: the order to remove
 *   index_node: pointer to next order to match
 *   product_node: product_info for the product orderbook
 */
void remove_match_node(struct order_type *match_node, struct order_type **index_node, struct product_info *product_node)
{
	struct order_type *match_order_before = match_node->prev;
	if (match_order_before == NULL && match_node->next != NULL)
	{
		product_node->first_order = match_node->next;
		match_node->next->prev = NULL;
	}
	else if (match_node->next != NULL)
	{
		match_order_before->next = match_node->next;
		match_node->next->prev = match_order_before;
	}
	else if (match_node->next == NULL)
	{
		product_node->first_order = NULL;
	}
	*index_node = (*index_node)->next;
	free(match_node->product);
	free(match_node);
}

/* Function: match_sell_equal_quan
 * 	----------------------------
 *   Match a sell order to a buy order that has the same quantity.
 *
 *   product_node: product_info for the product orderbook
 *   current_order: current order we want to match
 *   size: size of the product array
 *   index_node: pointer to next order to match
 *   match_node: the order we have matched against
 *   returns: the exchange fee for this fill
 */
long int match_sell_equal_quan(struct product_info *product_node, struct order_type *current_order, int size, struct order_type **index_node, struct order_type *match_node)
{
	long int exchange_fee = 0;
	struct trader_positions *buy_trader_positions = get_trader_positions(match_node->trader->positions, size, current_order->product);
	buy_trader_positions->quantity += current_order->quantity;
	long int quantity = current_order->price * current_order->quantity;
	buy_trader_positions->price -= quantity;
	struct trader_positions *sell_trader_positions = get_trader_positions(current_order->trader->positions, size, current_order->product);
	exchange_fee = custom_round(0.01 * (quantity));
	sell_trader_positions->quantity -= current_order->quantity;
	sell_trader_positions->price -= exchange_fee;
	sell_trader_positions->price += quantity;

	if (match_node->trader->alive)
	{
		send_fill(match_node->trader->fp_exchange_t, match_node->trader->pid_child, match_node->order_id, match_node->quantity);
	}

	send_fill(current_order->trader->fp_exchange_t, current_order->trader->pid_child, current_order->order_id, current_order->quantity);

	printf("%s Match: Order %d [T%d], New Order %d [T%d], value: $%ld, fee: $%ld.\n", LOG_PREFIX, match_node->order_id, match_node->trader->trader_id, current_order->order_id, current_order->trader->trader_id, quantity, exchange_fee);
	free(current_order->product);
	free(current_order);

	product_node->buy -= 1;
	remove_match_node(match_node, index_node, product_node);
	return exchange_fee;
}

/* Function: match_buy_equal_quan
 * 	----------------------------
 *   Match a buy order to a sell order that has the same quantity.
 *
 *   product_node: product_info for the product orderbook
 *   current_order: current order we want to match
 *   size: size of the product array
 *   index_node: pointer to next order to match
 *   match_node: the order we have matched against
 *   returns: the exchange fee for this fill
 */
long int match_buy_equal_quan(struct product_info *product_node, struct order_type *current_order, int size, struct order_type **index_node, struct order_type *match_node)
{
	long int exchange_fee = 0;
	struct trader_positions *buy_trader_positions = get_trader_positions(current_order->trader->positions, size, current_order->product);
	buy_trader_positions->quantity += current_order->quantity;
	long int quantity = match_node->price * match_node->quantity;
	buy_trader_positions->price -= quantity;
	struct trader_positions *sell_trader_positions = get_trader_positions(match_node->trader->positions, size, current_order->product);
	exchange_fee = custom_round(0.01 * (quantity));
	sell_trader_positions->quantity -= current_order->quantity;
	buy_trader_positions->price -= exchange_fee;
	sell_trader_positions->price += quantity;

	send_fill(current_order->trader->fp_exchange_t, current_order->trader->pid_child, current_order->order_id, current_order->quantity);

	if (match_node->trader->alive)
	{
		send_fill(match_node->trader->fp_exchange_t, match_node->trader->pid_child, match_node->order_id, match_node->quantity);
	}

	printf("%s Match Order: %d [T%d], New Order %d [T%d], value: $%ld, fee: $%ld.\n", LOG_PREFIX, current_order->order_id, current_order->trader->trader_id, match_node->order_id, match_node->trader->trader_id, quantity, exchange_fee);
	free(current_order->product);
	free(current_order);

	product_node->sell -= 1;
	remove_match_node(match_node, index_node, product_node);
	return exchange_fee;
}

/* Function: match_sell_buy_bigger
 * 	----------------------------
 *   Match a sell order to a buy order that has a bigger quantity than the sell order.
 *
 *   product_node: product_info for the product orderbook
 *   current_order: current order we want to match
 *   size: size of the product array
 *   index_node: pointer to next order to match
 *   match_node: the order we have matched against
 *   returns: the exchange fee for this fill
 */
long int match_sell_buy_bigger(struct product_info *product_node, struct order_type *current_order, int size, struct order_type **index_node, struct order_type *match_node)
{
	long int exchange_fee = 0;
	struct trader_positions *buy_trader_positions = get_trader_positions(match_node->trader->positions, size, current_order->product);
	buy_trader_positions->quantity += current_order->quantity;
	long int quantity = match_node->price * current_order->quantity;
	buy_trader_positions->price -= quantity;
	match_node->quantity -= current_order->quantity;
	struct trader_positions *sell_trader_positions = get_trader_positions(current_order->trader->positions, size, current_order->product);
	exchange_fee = custom_round(0.01 * (quantity));
	sell_trader_positions->quantity -= current_order->quantity;
	sell_trader_positions->price -= exchange_fee;
	sell_trader_positions->price += quantity;
	if (match_node->trader->alive)
	{
		send_fill(match_node->trader->fp_exchange_t, match_node->trader->pid_child, match_node->order_id, current_order->quantity);
	}
	send_fill(current_order->trader->fp_exchange_t, current_order->trader->pid_child, current_order->order_id, current_order->quantity);

	printf("%s Match: Order %d [T%d], New Order %d [T%d], value: $%ld, fee: $%ld.\n", LOG_PREFIX, match_node->order_id, match_node->trader->trader_id, current_order->order_id, current_order->trader->trader_id, quantity, exchange_fee);
	free(current_order->product);
	free(current_order);
	*index_node = (*index_node)->next;
	return exchange_fee;
}

/* Function: match_buy_sell_bigger
 * 	----------------------------
 *   Match a buy order to a sell order that has a bigger quantity than the buy order.
 *
 *   product_node: product_info for the product orderbook
 *   current_order: current order we want to match
 *   size: size of the product array
 *   index_node: pointer to next order to match
 *   match_node: the order we have matched against
 *   returns: the exchange fee for this fill
 */
long int match_buy_sell_bigger(struct product_info *product_node, struct order_type *current_order, int size, struct order_type **index_node, struct order_type *match_node)
{
	long int exchange_fee = 0;
	struct trader_positions *buy_trader_positions = get_trader_positions(current_order->trader->positions, size, current_order->product);
	buy_trader_positions->quantity += current_order->quantity;
	long int quantity = match_node->price * current_order->quantity;
	buy_trader_positions->price -= quantity;
	match_node->quantity -= current_order->quantity;
	struct trader_positions *sell_trader_positions = get_trader_positions(match_node->trader->positions, size, match_node->product);
	exchange_fee = custom_round(0.01 * (quantity));
	sell_trader_positions->quantity -= current_order->quantity;
	buy_trader_positions->price -= exchange_fee;
	sell_trader_positions->price += quantity;
	send_fill(current_order->trader->fp_exchange_t, current_order->trader->pid_child, current_order->order_id, current_order->quantity);

	printf("%s Match: Order %d [T%d], New Order %d [T%d], value: $%ld, fee: $%ld.\n", LOG_PREFIX, match_node->order_id, match_node->trader->trader_id, current_order->order_id, current_order->trader->trader_id, quantity, exchange_fee);

	if (match_node->trader->alive)
	{

		send_fill(match_node->trader->fp_exchange_t, match_node->trader->pid_child, match_node->order_id, current_order->quantity);
	}

	free(current_order->product);
	free(current_order);
	*index_node = (*index_node)->next;
	return exchange_fee;
}

/* Function: match_sell_sell_bigger
 * 	----------------------------
 *   Match a sell order to a buy where the sell order has a bigger quantity than the buy order.
 *
 *   product_node: product_info for the product orderbook
 *   current_order: current order we want to match
 *   size: size of the product array
 *   index_node: pointer to next order to match
 *   match_node: the order we have matched against
 *   returns: the exchange fee for this fill
 */
long int match_sell_sell_bigger(struct product_info *product_node, struct order_type *current_order, int size, struct order_type **index_node, struct order_type *match_node)
{
	long int exchange_fee = 0;
	struct trader_positions *buy_trader_positions = get_trader_positions(match_node->trader->positions, size, current_order->product);
	buy_trader_positions->quantity += match_node->quantity;
	long int quantity = match_node->price * match_node->quantity;
	buy_trader_positions->price -= quantity;
	struct trader_positions *sell_trader_positions = get_trader_positions(current_order->trader->positions, size, current_order->product);
	exchange_fee = custom_round(0.01 * (quantity));
	sell_trader_positions->quantity -= match_node->quantity;
	sell_trader_positions->price -= exchange_fee;
	sell_trader_positions->price += quantity;
	current_order->quantity -= match_node->quantity;
	if (match_node->trader->alive)
	{
		send_fill(match_node->trader->fp_exchange_t, match_node->trader->pid_child, match_node->order_id, match_node->quantity);
	}

	send_fill(current_order->trader->fp_exchange_t, current_order->trader->pid_child, current_order->order_id, match_node->quantity);

	printf("%s Match: Order %d [T%d], New Order %d [T%d], value: $%ld, fee: $%ld.\n", LOG_PREFIX, match_node->order_id, match_node->trader->trader_id, current_order->order_id, current_order->trader->trader_id, quantity, exchange_fee);
	if (match_node->level == 1)
	{
		product_node->buy -= 1;
	}
	remove_match_node(match_node, index_node, product_node);
	return exchange_fee;
}

/* Function: match_buy_buy_bigger
 * 	----------------------------
 *   Match a buy order to a sell where the buy order has a bigger quantity than the sell order.
 *
 *   product_node: product_info for the product orderbook
 *   current_order: current order we want to match
 *   size: size of the product array
 *   index_node: pointer to next order to match
 *   match_node: the order we have matched against
 *   returns: the exchange fee for this fill
 */
long int match_buy_buy_bigger(struct product_info *product_node, struct order_type *current_order, int size, struct order_type **index_node, struct order_type *match_node)
{
	long int exchange_fee = 0;
	struct trader_positions *buy_trader_positions = get_trader_positions(current_order->trader->positions, size, current_order->product);
	buy_trader_positions->quantity += match_node->quantity;
	long int quantity = match_node->price * match_node->quantity;
	buy_trader_positions->price -= quantity;
	struct trader_positions *sell_trader_positions = get_trader_positions(match_node->trader->positions, size, current_order->product);
	exchange_fee = custom_round(0.01 * (quantity));
	sell_trader_positions->quantity -= match_node->quantity;
	buy_trader_positions->price -= exchange_fee;
	sell_trader_positions->price += quantity;
	current_order->quantity -= match_node->quantity;

	send_fill(current_order->trader->fp_exchange_t, current_order->trader->pid_child, current_order->order_id, match_node->quantity);

	if (match_node->trader->alive)
	{
		send_fill(match_node->trader->fp_exchange_t, match_node->trader->pid_child, match_node->order_id, match_node->quantity);
	}
	printf("%s Match: Order %d [T%d], New Order %d [T%d], value: $%ld, fee: $%ld.\n", LOG_PREFIX, match_node->order_id, match_node->trader->trader_id, current_order->order_id, current_order->trader->trader_id, quantity, exchange_fee);
	if (match_node->level == 1)
	{
		product_node->sell -= 1;
	}
	remove_match_node(match_node, index_node, product_node);
	return exchange_fee;
}

/* Function: process_sell_order
 * 	----------------------------
 *   Find a potential match for the current order that is a sell order and then process it.
 *
 *   product_node: product_info for the product orderbook
 *   current_order: current order we want to match
 *   size: size of the product array
 *   returns: exchange fee for the order if matched
 */
long int process_sell_order(struct product_info *product_node, struct order_type *current_order, int size)
{

	struct order_type *index_node = product_node->first_order;
	long int exchange_fee = 0;
	int place_order = FALSE;
	int straight_insert = FALSE;
	if (product_node->buy == 0)
	{
		straight_insert = TRUE;
	}

	if (product_node->buy != 0)
	{
		size_t i;
		for (i = 0; i < product_node->sell; i++)
		{
			index_node = index_node->next;
		}

		while (index_node != NULL)
		{
			struct order_type *match_node = index_node;
			if ((match_node->quantity == current_order->quantity) && (match_node->price >= current_order->price))
			{
				exchange_fee += match_sell_equal_quan(product_node, current_order, size, &index_node, match_node);
				place_order = FALSE;
				break;
			}
			else if ((match_node->quantity > current_order->quantity) && (match_node->price >= current_order->price))
			{
				exchange_fee += match_sell_buy_bigger(product_node, current_order, size, &index_node, match_node);
				place_order = FALSE;
				break;
			}
			else if ((match_node->quantity < current_order->quantity) && (match_node->price >= current_order->price))
			{
				exchange_fee += match_sell_sell_bigger(product_node, current_order, size, &index_node, match_node);
				place_order = TRUE;
			}
			else
			{
				index_node = index_node->next;
				place_order = TRUE;
			}
		}
	}
	// No match: add to order linked list
	if (place_order || straight_insert)
	{
		if (product_node->sell == 0)
		{
			if (product_node->buy != 0)
			{
				product_node->first_order->prev = current_order;
				current_order->next = product_node->first_order;
			}
			product_node->first_order = current_order;
		}
		else
		{
			struct order_type *node = product_node->first_order;
			struct order_type *prev_node = NULL;
			while (node != NULL && node->type == SELL)
			{
				if (node->price > current_order->price)
				{
					if (node->prev != NULL)
					{
						node->prev->next = current_order;
						current_order->prev = node->prev;
					}
					if (node->prev == NULL)
					{
						product_node->first_order = current_order;
					}
					node->prev = current_order;
					current_order->next = node;
					break;
				}
				prev_node = node;
				node = node->next;
			}

			if (node == NULL || node->type == BUY)
			{
				current_order->prev = prev_node;
				prev_node->next = current_order;
			}

			if (node != NULL && node->type == BUY)
			{

				current_order->next = node;
				node->prev = current_order;
			}
		}
		product_node->sell += 1;
	}

	return exchange_fee;
}

/* Function: process_buy_order
 * 	----------------------------
 *   Find a potential match for the current order that is a buy order and then process it.
 *
 *   product_node: product_info for the product orderbook
 *   current_order: current order we want to match
 *   size: size of the product array
 *   returns: exchange fee for the order if matched
 */
long int process_buy_order(struct product_info *product_node, struct order_type *current_order, int size)
{

	struct order_type *index_node = product_node->first_order;
	long int exchange_fee = 0;
	int place_order = FALSE;
	int straight_insert = FALSE;
	if (product_node->sell == 0)
	{
		straight_insert = TRUE;
	}
	else if (product_node->sell != 0)
	{
		while (index_node != NULL && index_node->type != BUY)
		{
			struct order_type *match_node = index_node;
			if ((match_node->quantity == current_order->quantity) && (match_node->price <= current_order->price))
			{
				exchange_fee += match_buy_equal_quan(product_node, current_order, size, &index_node, match_node);
				place_order = FALSE;
				break;
			}
			else if ((match_node->quantity > current_order->quantity) && (match_node->price <= current_order->price))
			{
				exchange_fee += match_buy_sell_bigger(product_node, current_order, size, &index_node, match_node);
				place_order = FALSE;
				break;
			}
			else if ((match_node->quantity < current_order->quantity) && (match_node->price <= current_order->price))
			{
				exchange_fee += match_buy_buy_bigger(product_node, current_order, size, &index_node, match_node);
				place_order = TRUE;
			}
			else
			{
				index_node = index_node->next;
				place_order = TRUE;
			}
		}
	}
	// No match: add to order linked list
	if (place_order || straight_insert)
	{

		struct order_type *node = product_node->first_order;
		if (node != NULL && node->type != BUY)
		{
			while (node->type != BUY && node->next != NULL)
			{
				node = node->next;
			}
		}
		struct order_type *prev_node = NULL;
		while (node != NULL)
		{
			if (node->price < current_order->price)
			{
				if (node->prev != NULL)
				{
					node->prev->next = current_order;
					current_order->prev = node->prev;
				}
				if (node->prev == NULL)
				{
					product_node->first_order = current_order;
				}
				node->prev = current_order;
				current_order->next = node;
				break;
			}
			prev_node = node;
			node = node->next;
		}

		if (prev_node == NULL && node == product_node->first_order)
		{
			product_node->first_order = current_order;
		}

		else if (node == NULL)
		{
			prev_node->next = current_order;
			current_order->prev = prev_node;
		}
		product_node->buy++;
	}

	return exchange_fee;
}

/* Function: match_order
 * 	----------------------------
 *   Match an order depending on whether it's a buy or sell order.
 *
 *   product_node: product_info for the product orderbook
 *   current_order: current order we want to match
 *   size: size of the product array
 *   returns: exchange fee for the order if matched
 */
long int match_order(struct product_info *product_node, struct order_type *current_order, int size)
{
	long int exchange_fee = 0;
	if (current_order->type == SELL)
	{
		exchange_fee = process_sell_order(product_node, current_order, size);
	}
	else
	{
		exchange_fee = process_buy_order(product_node, current_order, size);
	}
	return exchange_fee;
}

/* Function: get_type
 * 	----------------------------
 *   Returns a string "BUY" or "SELL" depending on the type.
 *
 *   type: type to compare against
 *   returns: A string corresponding to the encoded value
 */
char *get_type(int type)
{
	if (type == SELL)
	{
		return ("SELL");
	}

	return ("BUY");
}

/* Function: print_order_positions
 * 	----------------------------
 *   Prints the orderbook and positions for each trader.
 *
 *   order_book: the orderbook array
 *   product_array: the array that stores the products as strings
 *   size: size of the product array
 *   number traders: the number of traders
 *   exchange_traders: linked list of traders
 */
void print_order_positions(struct product_info *order_book, char **product_array, int size, int number_traders, struct trader_struct *exchange_traders)
{
	printf("%s\t--ORDERBOOK--\n", LOG_PREFIX);

	for (int i = 0; i < size; i++)
	{

		int buy_level = 0;
		int sell_level = 0;

		struct order_type *current_order = order_book[i].first_order;

		while (current_order != NULL)
		{
			// printf("%ld\n", current_order->price);
			if (current_order->type == BUY && !(current_order->next != NULL && current_order->next->type == BUY && current_order->next->price == current_order->price))
			{
				buy_level++;
			}
			else if (current_order->type == SELL && !(current_order->next != NULL && current_order->next->type == SELL && current_order->next->price == current_order->price))
			{
				sell_level++;
			}
			current_order = current_order->next;
		}

		printf("%s\tProduct: %s; Buy levels: %d; Sell levels: %d\n", LOG_PREFIX, product_array[i], buy_level, sell_level);

		current_order = order_book[i].first_order;
		struct order_type *prev_order = NULL;

		long int number_orders = 0;
		long int level_quantity = 0;

		struct order_type *buy_start = order_book[i].first_order;

		// Go to the end of the sell list
		if (current_order != NULL && current_order->type == SELL)
		{
			for (int j = 0; current_order->next != NULL && current_order->next->type == SELL; j++)
			{
				current_order = current_order->next;
			}
			buy_start = current_order->next;

			while (current_order != NULL)
			{

				number_orders += 1;
				if (prev_order != NULL)
				{

					if (prev_order->type != current_order->type || prev_order->price != current_order->price)
					{
						char *type = get_type(prev_order->type);

						if (number_orders - 1 == 1)
						{
							printf("%s\t\t%s %ld @ $%ld (1 order)\n", LOG_PREFIX, type, level_quantity, prev_order->price);
						}
						else
						{
							printf("%s\t\t%s %ld @ $%ld (%ld orders)\n", LOG_PREFIX, type, level_quantity, prev_order->price, number_orders - 1);
						}

						number_orders = 1;
						level_quantity = 0;
					}
				}

				if (current_order->prev == NULL)
				{
					char *type = get_type(current_order->type);

					if (number_orders == 1)
					{
						printf("%s\t\t%s %ld @ $%ld (1 order)\n", LOG_PREFIX, type, current_order->quantity, current_order->price);
					}
					else
					{
						printf("%s\t\t%s %ld @ $%ld (%ld orders)\n", LOG_PREFIX, type, level_quantity + current_order->quantity, prev_order->price, number_orders);
					}
				}

				level_quantity += current_order->quantity;

				prev_order = current_order;
				current_order = current_order->prev;
			}
		}
		if (buy_start != NULL)
		{
			current_order = buy_start;
			prev_order = NULL;
			number_orders = 0;
			level_quantity = 0;
			while (current_order != NULL)
			{
				number_orders += 1;
				if (prev_order != NULL)
				{

					if (prev_order->type != current_order->type || prev_order->price != current_order->price)
					{
						char *type = get_type(prev_order->type);

						if (number_orders - 1 == 1)
						{
							printf("%s\t\t%s %ld @ $%ld (1 order)\n", LOG_PREFIX, type, level_quantity, prev_order->price);
						}
						else
						{
							printf("%s\t\t%s %ld @ $%ld (%ld orders)\n", LOG_PREFIX, type, level_quantity, prev_order->price, number_orders - 1);
						}

						number_orders = 1;
						level_quantity = 0;
					}
				}

				if (current_order->next == NULL)
				{
					char *type = get_type(current_order->type);

					if (number_orders == 1)
					{
						printf("%s\t\t%s %ld @ $%ld (1 order)\n", LOG_PREFIX, type, current_order->quantity, current_order->price);
					}
					else
					{
						printf("%s\t\t%s %ld @ $%ld (%ld orders)\n", LOG_PREFIX, type, level_quantity + current_order->quantity, prev_order->price, number_orders);
					}
				}

				level_quantity += current_order->quantity;

				prev_order = current_order;
				current_order = current_order->next;
			}
		}
	}

	printf("%s\t--POSITIONS--\n", LOG_PREFIX);
	for (int i = 0; i < number_traders; i++)
	{
		printf("%s\tTrader %d: ", LOG_PREFIX, i);
		struct trader_positions *position = exchange_traders[i].positions;
		for (int j = 0; j < size; j++)
		{
			if (j == 0)
			{
				printf("%s %ld ($%ld)", position[j].product, position[j].quantity, position[j].price);
			}
			else
			{
				printf(", %s %ld ($%ld)", position[j].product, position[j].quantity, position[j].price);
			}
		}
		printf("\n");
	}
}

/* Function: get_order
 * 	----------------------------
 *   Removes the order from the orderbook and returns it.
 *
 *   trader_id: trader that wanted the order removed
 *   order_id: order id to remove
 *   order_book: the orderbook array
 *   size: size of the product array
 */
struct order_type *get_order(int trader_id, int order_id, struct product_info *order_book, int size)
{
	for (size_t i = 0; i < size; i++)
	{
		if (order_book[i].buy + order_book[i].sell == 0)
		{
			continue;
		}
		struct order_type *node = order_book[i].first_order;
		while (node != NULL)
		{
			if (node->trader->trader_id == trader_id && node->order_id == order_id)
			{
				if (node->prev == NULL)
				{
					order_book[i].first_order = node->next;
					if (node->next != NULL)
					{
						node->next->prev = NULL;
					}
				}
				else
				{
					if (node->next != NULL)
					{
						node->next->prev = node->prev;
					}
					node->prev->next = node->next;
				}

				if (node->type == SELL)
				{
					order_book[i].sell--;
				}
				else
				{
					order_book[i].buy--;
				}

				return node;
			}
			node = node->next;
		}
	}
	return NULL;
}

/* Function: process_cancel
 * 	----------------------------
 *   Removes the order from the order linked list.
 *
 *   buff_check: input of the command
 *   sent_id: pointer to the id of the trader that sent the command
 *   trader_id: trader that wanted the order removed
 *   order_id: order id to remove
 *   order_book: the orderbook array
 *   size: size of the product array
 *   trader: the trader with the order to cancel
 *   current_order: current order we want to match
 *   product_array: the array that stores the products as strings
 *   number traders: the number of traders
 *   exchange_traders: linked list of traders
 *   match: pointer to indicate whether to process buy/sell
 */
void process_cancel(char *buff_check, int *sent_id, struct trader_struct *trader, struct order_type *current_order, struct trader_struct *exchange_traders, char **product_array, int number_traders, struct product_info *order_book, int *match, int size, char *order_id)
{
	char *check_next = strsep(&buff_check, " ");
	if (check_next != NULL)
	{
		send_invalid(trader->fp_exchange_t, trader->pid_child);
	}
	else
	{

		current_order = get_order(*sent_id, atoi(order_id), order_book, size);
		if (current_order != NULL)
		{

			send_cancel(trader->fp_exchange_t, trader->pid_child, atoi(order_id));
			char *type = get_type(current_order->type);

			for (size_t i = 0; i < number_traders; i++)
			{
				if (&(exchange_traders[i]) != current_order->trader && exchange_traders[i].alive)
				{
					fprintf(exchange_traders[i].fp_exchange_t, "MARKET %s %s 0 0;", type, current_order->product);
					fflush(exchange_traders[i].fp_exchange_t);
					kill(exchange_traders[i].pid_child, SIGUSR1);
				}
			}
			print_order_positions(order_book, product_array, size, number_traders, exchange_traders);
			free(current_order->product);
			free(current_order);
			*match = FALSE;
		}
	}
}

/* Function: send_market_signals
 * 	----------------------------
 *   Sends accepted or amend.
 *
 *   current_order: current order we want to match
 *   match: pointer to indicate whether the order is of type 'accept' or 'amend'
 */
void send_market_signals(int *append, struct order_type *current_order, struct trader_struct *exchange_traders, int number_traders)
{
	if (*append == FALSE)
	{
		fprintf(current_order->trader->fp_exchange_t, "ACCEPTED %d;", current_order->order_id);
	}
	else
	{
		fprintf(current_order->trader->fp_exchange_t, "AMENDED %d;", current_order->order_id);
	}
	fflush(current_order->trader->fp_exchange_t);
	kill(current_order->trader->pid_child, SIGUSR1);

	char *type = get_type(current_order->type);
	for (size_t i = 0; i < number_traders; i++)
	{
		if (&(exchange_traders[i]) != current_order->trader && exchange_traders[i].alive)
		{
			fprintf(exchange_traders[i].fp_exchange_t, "MARKET %s %s %ld %ld;", type, current_order->product, current_order->quantity, current_order->price);
			fflush(exchange_traders[i].fp_exchange_t);
			kill(exchange_traders[i].pid_child, SIGUSR1);
		}
	}
}

/* Function: process_matching
 * 	----------------------------
 *   Processes the order matching.
 *
 *   order_book: the orderbook array
 *   product_array: the array that stores the products as strings
 *   size: size of the product array
 *   number traders: the number of traders
 *   exchange_traders: linked list of traders
 *   current_order: current order we want to match
 *   returns: exchange fee for the order
 */
long int process_matching(struct product_info *order_book, char **product_array, int size, int number_traders, struct trader_struct *exchange_traders, struct order_type *current_order)
{
	long int exchange_fee = 0;
	for (size_t i = 0; i < size; i++)
	{
		if (strcmp(product_array[i], current_order->product) == 0)
		{
			if (order_book[i].first_order == NULL)
			{
				order_book[i].first_order = current_order;
				update_product_info(current_order->type, &(order_book[i]));
				current_order->prev = NULL;
			}
			else
			{
				current_order->prev = NULL;
				current_order->next = NULL;
				exchange_fee += match_order(&order_book[i], current_order, size);
			}
			break;
		}
	}
	return exchange_fee;
}

/* Function: manage_disconnect
 * 	----------------------------
 *   Manages the messages for disconnected traders.
 *
 *   filedes: poll file description
 *   number traders: the number of traders
 *   exchange_traders: linked list of traders
 *   returns: number of disconnected traders
 */
int manage_disconnect(struct pollfd *filedes, int number_traders, struct trader_struct *exchange_traders)
{
	int dead_children = 0;
	poll(filedes, number_traders, -1);

	if (!pipe_signal && old_disconnect == disconnect_child)
	{
		for (int i = 0; i < number_traders; i++)
		{
			if (filedes[i].revents & POLLHUP && exchange_traders[i].alive)
			{
				printf("%s Trader %d disconnected\n", LOG_PREFIX, i);
				exchange_traders[i].alive = FALSE;
				kill(exchange_traders[i].pid_child, SIGKILL);
				dead_children++;
			}
		}
	}
	// Child disconnect
	if (old_disconnect != disconnect_child)
	{
		int id = get_id_pid(disconnect_child, exchange_traders, number_traders);
		if (exchange_traders[id].alive == TRUE)
		{
			printf("%s Trader %d disconnected\n", LOG_PREFIX, id);
			exchange_traders[id].alive = FALSE;
			dead_children++;
		}
		old_disconnect = disconnect_child;
	}
	return dead_children;
}

#ifndef TESTING

int main(int argc, char **argv)
{
	// --------------------INITIALISATION----------------------------
	printf("%s Starting\n", LOG_PREFIX);

	int number_traders = argc - 2;
	int size = get_products_size(argv[1]);
	long int exchange_fee = 0;

	char **product_array = load_products_file(argv[1]);
	print_trading(product_array, size);

	struct trader_struct *exchange_traders = malloc(sizeof(struct trader_struct) * number_traders);
	initalise_traders(argv, argc, exchange_traders, size, product_array);

	market_open(number_traders, exchange_traders);

	struct sigaction te_sign;
	memset(&te_sign, 0, sizeof(struct sigaction));
	te_sign.sa_sigaction = te_sig;
	te_sign.sa_flags = SA_SIGINFO;
	if (sigaction(SIGUSR1, &te_sign, NULL) == -1)
	{
		perror("sigaction failed SIGUSR1");
		return 1;
	}

	struct sigaction child_sign;
	memset(&child_sign, 0, sizeof(struct sigaction));
	child_sign.sa_sigaction = child_sig;
	child_sign.sa_flags = SA_SIGINFO;
	if (sigaction(SIGCHLD, &child_sign, NULL) == -1)
	{
		perror("sigaction failed SIGCHLD");
		return 1;
	}

	struct product_info *order_book = malloc(sizeof(struct product_info) * size);
	for (size_t i = 0; i < size; i++)
	{
		order_book[i].buy = 0;
		order_book[i].sell = 0;
		order_book[i].first_order = NULL;
	}

	struct pollfd *filedes = malloc(sizeof(struct pollfd) * number_traders);

	for (int i = 0; i < number_traders; i++)
	{
		filedes[i].fd = exchange_traders[i].trader_fd;
		filedes[i].events = 0;
	}
	int dead_children = 0;

	// --------------------PROCESSING----------------------------
	while (dead_children < number_traders)
	{
		if (!pipe_signal)
		{
			dead_children += manage_disconnect(filedes, number_traders, exchange_traders);
		}
		if (pipe_signal)
		{
			pipe_signal = FALSE;
			int id = 0;
			int *sent_id = &id;
			FILE *pipe = get_trader_e_fp(trader_sig_id, exchange_traders, number_traders, sent_id);
			char *buff = varstin(pipe);
			int match_val = TRUE;
			int *match = &match_val;
			int append_val = FALSE;
			int *append = &append_val;
			char *buff_check = malloc(sizeof(char) * strlen(buff) + 1);
			strcpy(buff_check, buff);
			char *buff_check_ptr = buff_check;
			struct order_type *current_order = NULL;

			printf("%s [T%d] Parsing command: <%s>\n", LOG_PREFIX, *sent_id, buff);
			char *command = strsep(&buff_check, " ");
			// --------------------AMEND AND CANCEL----------------------------
			if (strcmp(command, "AMEND") == 0 || strcmp(command, "CANCEL") == 0)
			{
				struct trader_struct *trader = get_trader_id(*sent_id, exchange_traders, size);
				char *order_id = strsep(&buff_check, " ");
				if (order_id != NULL)
				{

					if (strcmp(command, "CANCEL") == 0)
					{
						process_cancel(buff_check, sent_id, trader, current_order, exchange_traders, product_array, number_traders, order_book, match, size, order_id);
					}
					else if (strcmp(command, "AMEND") == 0)
					{
						char *sep = strsep(&buff_check, " ");
						if (sep != NULL)
						{
							long int quantity = atoi(sep);
							if (quantity > 0 && quantity < UPPER_BOUND)
							{
								sep = strsep(&buff_check, " ");
								if (sep != NULL)
								{
									long int price = atoi(sep);
									if (price > 0 && price < UPPER_BOUND)
									{
										current_order = get_order(*sent_id, atoi(order_id), order_book, size);
										if (current_order != NULL)
										{
											current_order->price = price;
											current_order->quantity = quantity;
											*match = TRUE;
											*append = TRUE;
										}
									}
								}
							}
						}
						else
						{
							send_invalid(trader->fp_exchange_t, trader->pid_child);
						}
					}
				}
			}
			// --------------------BUY AND SELL----------------------------
			if (*match)
			{
				if (*append == FALSE)
				{
					current_order = make_current_order(size, number_traders, product_array, buff, *sent_id, exchange_traders);
				}
				if (current_order != NULL)
				{
					send_market_signals(append, current_order, exchange_traders, number_traders);
					exchange_fee += process_matching(order_book, product_array, size, number_traders, exchange_traders, current_order);
					print_order_positions(order_book, product_array, size, number_traders, exchange_traders);
				}
			}

			free(buff);
			free(buff_check_ptr);
		}
	}
	wait(NULL);

	// --------------------FREEING----------------------------
	free_traders(number_traders, exchange_traders);
	free_order_book(order_book, size);
	free_product_array(size, product_array);
	free(filedes);

	printf("%s Trading completed\n", LOG_PREFIX);
	printf("%s Exchange fees collected: $%ld\n", LOG_PREFIX, exchange_fee);

	return 0;
}

#endif