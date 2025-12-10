# INSPECTION
#
# inspecting/accessing objects at runtime
class Dynamic:
    def __getattr__(self, name):
        return f"You asked for {name}"
d = Dynamic()
print("(1)", d.unknown)
# METACLASSES
class ModelMeta(type):
    models = {}

    def __new__(mcls, name, bases, ns):
        cls = super().__new__(mcls, name, bases, ns)
        if name != "Model":
            ModelMeta.models[name] = cls
        return cls

class Model(metaclass=ModelMeta):
    pass
class User(Model):
    pass
class Order(Model):
    pass
print("(2)", ModelMeta.models)
# DECORATORS
def log(fn):
    def wrapper(*a, **kw):
        print("(3) CALL", fn.__name__)
        return fn(*a, **kw)
    return wrapper

@log
def func():
    pass
func()
# MONKEY PATCHING
#
# Replacing behavior at runtime
import math
math.sqrt = lambda x: 42
print("(4)", math.sqrt(9))
# Code generation & execution
code = "def add(a,b): return a+b"
env = {}
exec(code, env)
print("(5)", env['add'](2, 3))
