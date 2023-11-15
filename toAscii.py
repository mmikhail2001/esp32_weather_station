# Открываем файл для чтения в бинарном режиме
with open('index.html', 'rb') as file:
    # Читаем все байты из файла
    file_bytes = file.read()

# Функция для преобразования байта в формат 0xHH
def format_byte(byte):
    return '0x{:02X}'.format(byte)

# Преобразуем байты в массив строк в формате "0xHH"
byte_array = [format_byte(byte) for byte in file_bytes]

# Разбиваем массив на строки по 15 байтов
lines = [byte_array[i:i+15] for i in range(0, len(byte_array), 15)]

# Выводим массив байтов в формате C-массива с переносами строк
print('{')
for line in lines:
    print('    ' + ', '.join(line) + ',')  # Добавляем запятую в конце каждой строки
print('};')