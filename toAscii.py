with open('index.html', 'rb') as file:
    file_bytes = file.read()

def format_byte(byte):
    return '0x{:02X}'.format(byte)

byte_array = [format_byte(byte) for byte in file_bytes]

lines = [byte_array[i:i+15] for i in range(0, len(byte_array), 15)]

print('{')
for line in lines:
    print('    ' + ', '.join(line) + ',')  # Добавляем запятую в конце каждой строки
print('};')