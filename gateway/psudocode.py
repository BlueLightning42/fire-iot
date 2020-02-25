from enum import Enum

class MsgType(Enum):
  heartbeat = auto()
  error = auto()
  alarm = auto()

class Error(Enum):
  cant_reach = auto()
  alarm = auto()

class msg:
	def msg(self, _typ, _id, _value):
		self.type = _typ;
		self.id = _id;
		self.value = _value

class Device:
	def Device(self):
		self.time_passed = 0



def poll_messages():
'''collect a list of all recived messages
  parse each relevant part of the message into msg objects
  check the validity of each message with its parity bit
  then return a list of messages'''
	pass

def send_error(id, err):
'''recive a error code and device
  check stored information about the device and send all relevant data to fire department'''
	pass

def init_device_store():
'''on initial setup reads device store from storage'''
  pass



timeout_time = 100
alarm_timeout = 1000

if __name__ == "__main__":
  running = True
  while running:
    messages = poll_messages()

    for message in messages:
      if message.type == MsgType.heartbeat:
        devices[message.id].time_passed = 0
      elif message.type == MsgType.error:
        send_error(message.id, message.value)

      if message.type == MsgType.alarm:
        devices[message.id].alarm_on += 1
      else:
        devices[message.id].alarm_on = 0
    
    for id, device in devices.items():
      if device.time_passed > timeout_time:
        send_error(id, Error.cant_reach)
      devices[id].time_passed += 1

      if devices[id].alarm_on > alarm_timeout:
        send_error(id, Error.alarm)

