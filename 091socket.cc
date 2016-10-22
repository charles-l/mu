:(before "End Types")
struct socket_t {
  int fd;
  sockaddr_in addr;
  bool polled;
  socket_t() {
    fd = 0;
    polled = false;
    bzero(&addr, sizeof(addr));
  }
};

:(before "End Primitive Recipe Declarations")
_OPEN_CLIENT_SOCKET,
:(before "End Primitive Recipe Numbers")
put(Recipe_ordinal, "$open-client-socket", _OPEN_CLIENT_SOCKET);
:(before "End Primitive Recipe Checks")
case _OPEN_CLIENT_SOCKET: {
  if (SIZE(inst.ingredients) != 2) {
    raise << maybe(get(Recipe, r).name) << "'$open-client-socket' requires exactly two ingredients, but got '" << inst.original_string << "'\n" << end();
    break;
  }
  if (!is_mu_text(inst.ingredients.at(0))) {
    raise << maybe(get(Recipe, r).name) << "first ingredient of '$open-client-socket' should be text (the hostname), but got '" << to_string(inst.ingredients.at(0)) << "'\n" << end();
    break;
  }
  if (!is_mu_number(inst.ingredients.at(1))) {
    raise << maybe(get(Recipe, r).name) << "second ingredient of '$open-client-socket' should be a number (the port of the hostname to connect to), but got '" << to_string(inst.ingredients.at(1)) << "'\n" << end();
    break;
  }
  if (SIZE(inst.products) != 1) {
    raise << maybe(get(Recipe, r).name) << "'$open-client-socket' requires exactly one product, but got '" << inst.original_string << "'\n" << end();
    break;
  }
  if (!is_mu_number(inst.products.at(0))) {
    raise << maybe(get(Recipe, r).name) << "first product of '$open-client-socket' should be a number (socket handle), but got '" << to_string(inst.products.at(0)) << "'\n" << end();
    break;
  }
  break;
}
:(before "End Primitive Recipe Implementations")
case _OPEN_CLIENT_SOCKET: {
  string host = read_mu_text(ingredients.at(0).at(0));
  int port = ingredients.at(1).at(0);
  socket_t* client = client_socket(host, port);
  products.resize(1);
  if (client->fd < 0) {  // error
    delete client;
    products.at(0).push_back(0);
    break;
  }
  long long int result = reinterpret_cast<long long int>(client);
  products.at(0).push_back(static_cast<double>(result));
  break;
}
:(code)
socket_t* client_socket(const string& host, int port) {
  socket_t* result = new socket_t;
  result->fd = socket(AF_INET, SOCK_STREAM, 0);
  if (result->fd < 0) {
    raise << "Failed to create socket.\n" << end();
    return result;
  }
  result->addr.sin_family = AF_INET;
  hostent* tmp = gethostbyname(host.c_str());
  bcopy(tmp->h_addr, reinterpret_cast<char*>(&result->addr.sin_addr.s_addr), tmp->h_length);
  result->addr.sin_port = htons(port);
  if (connect(result->fd, reinterpret_cast<sockaddr*>(&result->addr), sizeof(result->addr)) < 0) {
    close(result->fd);
    result->fd = -1;
    raise << "Failed to connect to " << host << ':' << port << '\n' << end();
  }
  return result;
}

:(before "End Primitive Recipe Declarations")
_OPEN_SERVER_SOCKET,
:(before "End Primitive Recipe Numbers")
put(Recipe_ordinal, "$open-server-socket", _OPEN_SERVER_SOCKET);
:(before "End Primitive Recipe Checks")
case _OPEN_SERVER_SOCKET: {
  if (SIZE(inst.ingredients) != 1) {
    raise << maybe(get(Recipe, r).name) << "'$open-server-socket' requires exactly one ingredient (the port to listen for requests on), but got '" << inst.original_string << "'\n" << end();
    break;
  }
  if (!is_mu_number(inst.ingredients.at(0))) {
    raise << maybe(get(Recipe, r).name) << "first ingredient of '$open-server-socket' should be a number, but got '" << to_string(inst.ingredients.at(0)) << "'\n" << end();
    break;
  }
  if (SIZE(inst.products) != 1) {
    raise << maybe(get(Recipe, r).name) << "'$open-server-socket' requires exactly one product, but got '" << inst.original_string << "'\n" << end();
    break;
  }
  if (!is_mu_number(inst.products.at(0))) {
    raise << maybe(get(Recipe, r).name) << "first product of '$open-server-socket' should be a number (file handle), but got '" << to_string(inst.products.at(0)) << "'\n" << end();
    break;
  }
  break;
}
:(before "End Primitive Recipe Implementations")
case _OPEN_SERVER_SOCKET: {
  int port = ingredients.at(0).at(0);
  socket_t* server = server_socket(port);
  products.resize(1);
  if (server->fd < 0) {
    delete server;
    products.at(0).push_back(0);
    break;
  }
  long long int result = reinterpret_cast<long long int>(server);
  products.at(0).push_back(static_cast<double>(result));
  break;
}
:(code)
socket_t* server_socket(int port) {
  socket_t* result = new socket_t;
  result->fd = socket(AF_INET, SOCK_STREAM, 0);
  if (result->fd < 0) {
    raise << "Failed to create server socket.\n" << end();
    return result;
  }
  int dummy = 0;
  setsockopt(result->fd, SOL_SOCKET, SO_REUSEADDR, &dummy, sizeof(dummy));
  result->addr.sin_family = AF_INET;
  result->addr.sin_addr.s_addr = INADDR_ANY;
  result->addr.sin_port = htons(port);
  if (bind(result->fd, reinterpret_cast<sockaddr*>(&result->addr), sizeof(result->addr)) >= 0) {
    listen(result->fd, /*queue length*/5);
  }
  else {
    close(result->fd);
    result->fd = -1;
    raise << "Failed to bind result socket to port " << port << ". Something's already using that port.\n" << end();
  }
  return result;
}

:(before "End Primitive Recipe Declarations")
_ACCEPT,
:(before "End Primitive Recipe Numbers")
put(Recipe_ordinal, "$accept", _ACCEPT);
:(before "End Primitive Recipe Checks")
case _ACCEPT: {
  if (SIZE(inst.ingredients) != 1) {
    raise << maybe(get(Recipe, r).name) << "'$accept' requires exactly one ingredient, but got '" << inst.original_string << "'\n" << end();
    break;
  }
  if (!is_mu_number(inst.ingredients.at(0))) {
    raise << maybe(get(Recipe, r).name) << "first ingredient of '$accept' should be a number, but got '" << to_string(inst.ingredients.at(0)) << "'\n" << end();
    break;
  }
  if (SIZE(inst.products) != 1) {
    raise << maybe(get(Recipe, r).name) << "'$accept' requires exactly one product, but got '" << inst.original_string << "'\n" << end();
    break;
  }
  if (!is_mu_number(inst.products.at(0))) {
    raise << maybe(get(Recipe, r).name) << "first product of '$accept' should be a number (file handle), but got '" << to_string(inst.products.at(0)) << "'\n" << end();
    break;
  }
  break;
}
:(before "End Primitive Recipe Implementations")
case _ACCEPT: {
  products.resize(2);
  products.at(1).push_back(ingredients.at(0).at(0));  // indicate it modifies its ingredient
  long long int x = static_cast<long long int>(ingredients.at(0).at(0));
  socket_t* server = reinterpret_cast<socket_t*>(x);
  if (server) {
    socket_t* session = accept_session(server);
    long long int result = reinterpret_cast<long long int>(session);
    products.at(0).push_back(static_cast<double>(result));
  }
  else {
    products.at(0).push_back(0);
  }
  break;
}
:(code)
socket_t* accept_session(socket_t* server) {
  if (server->fd == 0) return NULL;
  socket_t* result = new socket_t;
  socklen_t dummy = sizeof(result->addr);
  result->fd = accept(server->fd, reinterpret_cast<sockaddr*>(&result->addr), &dummy);
  return result;
}

:(before "End Primitive Recipe Declarations")
_READ_FROM_SOCKET,
:(before "End Primitive Recipe Numbers")
put(Recipe_ordinal, "$read-from-socket", _READ_FROM_SOCKET);
:(before "End Primitive Recipe Checks")
case _READ_FROM_SOCKET: {
  if (SIZE(inst.ingredients) != 2) {
    raise << maybe(get(Recipe, r).name) << "'$read-from-socket' requires exactly two ingredients, but got '" << inst.original_string << "'\n" << end();
    break;
  }
  if (!is_mu_number(inst.ingredients.at(0))) {
    raise << maybe(get(Recipe, r).name) << "first ingredient of '$read-from-socket' should be a number, but got '" << to_string(inst.ingredients.at(0)) << "'\n" << end();
    break;
  }
  if (!is_mu_number(inst.ingredients.at(1))) {
    raise << maybe(get(Recipe, r).name) << "second ingredient of '$read-from-socket' should be a number, but got '" << to_string(inst.ingredients.at(0)) << "'\n" << end();
    break;
  }
  if (SIZE(inst.products) != 2) {
    raise << maybe(get(Recipe, r).name) << "'$read-from-socket' requires exactly two product, but got '" << inst.original_string << "'\n" << end();
    break;
  }
  if (!is_mu_text(inst.products.at(0))) {
    raise << maybe(get(Recipe, r).name) << "first product of '$read-from-socket' should be a text (address array character), but got '" << to_string(inst.products.at(0)) << "'\n" << end();
    break;
  }
  if (!is_mu_boolean(inst.products.at(1))) {
    raise << maybe(get(Recipe, r).name) << "second product of '$read-from-socket' should be a boolean (eof?), but got '" << to_string(inst.products.at(1)) << "'\n" << end();
    break;
  }
  break;
}
:(before "End Primitive Recipe Implementations")
case _READ_FROM_SOCKET: {
  cerr << "$read-from-socket\n";
  long long int x = static_cast<long long int>(ingredients.at(0).at(0));
  socket_t* socket = reinterpret_cast<socket_t*>(x);
  // 1. we'd like to simply read() from the socket
  // however read() on a socket never returns EOF, so we wouldn't know when to stop
  // 2. recv() can signal EOF, but it also signals "no data yet" in the beginning
  // so use poll() in the beginning to wait for data before calling recv()
  // 3. but poll() will block on EOF, so only use poll() on the very first
  // $read-from-socket on a socket
  if (!socket->polled) {
    pollfd p;
    bzero(&p, sizeof(p));
    p.fd = socket->fd;
    p.events = POLLIN | POLLHUP;
    if (poll(&p, /*num pollfds*/1, /*timeout*/100/*ms*/) <= 0) {
      raise << maybe(current_recipe_name()) << "error in $read-from-socket\n" << end();
      products.resize(2);
      products.at(0).push_back(0);
      products.at(1).push_back(false);
      break;
    }
    cerr << "poll output: " << p.revents << '\n';
    cerr << "setting socket->polled\n";
    socket->polled = true;
  }
  cerr << "$read-from-socket " << x << " continuing\n";
  int bytes = static_cast<int>(ingredients.at(1).at(0));
  char* contents = new char[bytes];
  bzero(contents, bytes);
  int bytes_read = recv(socket->fd, contents, bytes-/*terminal null*/1, MSG_DONTWAIT);
  products.resize(2);
  products.at(0).push_back(new_mu_text(contents));
  products.at(1).push_back(bytes_read <= 0);
  delete contents;
  break;
}

:(before "End Primitive Recipe Declarations")
_WRITE_TO_SOCKET,
:(before "End Primitive Recipe Numbers")
put(Recipe_ordinal, "$write-to-socket", _WRITE_TO_SOCKET);
:(before "End Primitive Recipe Checks")
case _WRITE_TO_SOCKET: {
  if (SIZE(inst.ingredients) != 2) {
    raise << maybe(get(Recipe, r).name) << "'$write-to-socket' requires exactly two ingredient, but got '" << inst.original_string << "'\n" << end();
    break;
  }
  break;
}
:(before "End Primitive Recipe Implementations")
case _WRITE_TO_SOCKET: {
  long long int x = static_cast<long long int>(ingredients.at(0).at(0));
  socket_t* session = reinterpret_cast<socket_t*>(x);
  // write just one character at a time to the session socket
  long long int y = static_cast<long long int>(ingredients.at(1).at(0));
  char c = static_cast<char>(y);
  if (write(session->fd, &c, 1) != 1) {
    raise << maybe(current_recipe_name()) << "failed to write to socket\n" << end();
    exit(0);
  }
  long long int result = reinterpret_cast<long long int>(session);
  products.resize(1);
  products.at(0).push_back(result);
  break;
}

:(before "End Primitive Recipe Declarations")
_CLOSE_SOCKET,
:(before "End Primitive Recipe Numbers")
put(Recipe_ordinal, "$close-socket", _CLOSE_SOCKET);
:(before "End Primitive Recipe Checks")
case _CLOSE_SOCKET: {
  if (SIZE(inst.ingredients) != 1) {
    raise << maybe(get(Recipe, r).name) << "'$close-socket' requires exactly two ingredient, but got '" << inst.original_string << "'\n" << end();
    break;
  }
  if (!is_mu_number(inst.ingredients.at(0))) {
    raise << maybe(get(Recipe, r).name) << "first ingredient of '$close-socket' should be a character, but got '" << to_string(inst.ingredients.at(0)) << "'\n" << end();
    break;
  }
  break;
}
:(before "End Primitive Recipe Implementations")
case _CLOSE_SOCKET: {
  long long int x = static_cast<long long int>(ingredients.at(0).at(0));
  socket_t* socket = reinterpret_cast<socket_t*>(x);
  close(socket->fd);
  break;
}

:(before "End Includes")
#include <netinet/in.h>
#include <netdb.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>
