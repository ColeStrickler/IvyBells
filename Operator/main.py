import sys
import time

import colorama

from Gadgets.Operator import Operator
from colorama import Fore, Back, Style

try:
    operator = str(sys.argv[1])
    password = str(sys.argv[2])
    url = sys.argv[3]
    operator_endpoint = sys.argv[4]
except:
    print("\nUSAGE:\nOperator.py [username] [password] [server url] [operator endpoint]")
    sys.exit(-1)

colorama.init()






user = Operator(operator, password, url, operator_endpoint)
check = user.login()
if not check:
    exit(-1)
user.main_menu()

time.sleep(2)
time.sleep(1)
print("\n\n")

