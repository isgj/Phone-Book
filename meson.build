project(
  'phone book',
  'c',
  lisence : 'GPL3'
)

# The files the server depends on
server_src = [
  'src/server.c',
  'src/connection_handler.c',
  'src/worker.c',
  'src/gui.c'
]

# The files the client depends on
client_src = [
  'src/client.c',
  'src/client_menu.c',
  'src/client_forms.c',
  'src/client_connection.c'
]

# Linked libraries
pthread = '-lpthread'
openssl = '-lssl'
crypto = '-lcrypto'
ncurses = '-lncurses'
panel = '-lpanel'

# The CPU family
arch = build_machine.cpu_family()

# The name of the executables
server_name = '_'.join(['server', arch])
client_name = '_'.join(['client', arch])

# Create the executables
executable(server_name, server_src, link_args: [pthread, openssl, crypto, ncurses, panel])
executable(client_name, client_src, link_args: [ncurses])
