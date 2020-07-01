extends Node2D

var hello_world

func _ready():
	if Engine.has_singleton("Uvc"):
		hello_world = Engine.get_singleton("Uvc")


func _on_Button_pressed():
	if hello_world:
		print(hello_world.helloWorld())
