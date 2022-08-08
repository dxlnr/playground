from model import CNN
from torch_utils import get_model_dtype, get_torch_model_shape


model = CNN()
dtype = get_model_dtype(model)
print(dtype)
print(get_torch_model_shape(model))
