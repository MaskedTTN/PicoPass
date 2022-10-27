import json

with open('config.json', 'rb')as file:
    data = json.load(file)
    file.close()

print(data)