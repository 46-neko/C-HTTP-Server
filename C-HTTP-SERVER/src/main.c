#include <http_server.h>

#define PORT "8080"
#define ADDRESS "localhost"

int main()
{
	serve(ADDRESS, PORT);

	return 0;
}
