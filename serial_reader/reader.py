import serial

try:
    with serial.Serial('COM5', baudrate=115200, timeout=1) as ser:
        print(f"Listening on {ser.name}...")
        while True:
            line = ser.readline().decode('utf-8').strip()
            if line:
                print(line)
except serial.SerialException as e:
    print(f"Error: {e}")
except KeyboardInterrupt:
    print("\nSerial monitoring stopped")
