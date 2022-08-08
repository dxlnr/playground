from typing import List

import numpy as np
import torch


def all_equal_list(iterator: List[any]) -> bool:
    r"""
    :param iterator: Input list object.
    :returns: Returns the element if all elements in the list are equal otherwise None.
    """
    iterator = iter(iterator)
    try:
        first = next(iterator)
    except StopIteration:
        return True
    if all(first == x for x in iterator):
        return first
    else:
        return None


def translate_model_dtype(torch_type: str) -> str:
    r"""."""
    if torch_type == "torch.float32" or "torch.float":
        _dtype = "F32"
    elif torch_type == "torch.float64" or "torch.double":
        _dtype = "F64"
    else:
        raise ValueError(
            f"{torch_type} is not supported by aggregation server. \
            Federation will fail. Please use 'torch.float' or 'torch.double'."
        )
    return _dtype


def get_model_dtype(model: torch.nn.Module) -> str:
    r"""."""
    if isinstance(model, torch.nn.Module):
        dtype_list = [layer[1].dtype for layer in list(model.state_dict().items())]
        if torch_type := all_equal_list(dtype_list):
            return translate_model_dtype(torch_type)
        else:
            raise ValueError(f"Found inconsistent data type for {model}. Mixed precision policy is not allowed at this point.\
             Consider reimplementing model with consistent data type above all layers.")
    else:
        raise TypeError(f"Unknown model type: {type(model)}. Consider to inherite from torch.nn.Module.")


def get_torch_model_shape(model: torch.nn.Module) -> List[np.ndarray]:
    r"""Extracts the shape of the pytorch model.

    :param model: Pytorch model object.
    :returns: List of np.ndarray which contains the shape (size) of each individual layer of the model.
    """
    return [
        np.array(model.state_dict()[param_tensor].size())
        for param_tensor in model.state_dict().keys()
    ]
