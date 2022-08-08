from typing import List, Any

import numpy as np
import tensorflow as tf


def get_shape(weights) -> List[Any]:
    r"""Reads in the weights and returns its shape as a list object."""
    return [np.array(layer.shape) for layer in weights]


def translate_tf_model_dtype(tf_type: str) -> str:
    r"""."""
    if tf_type == "float32":
        _dtype = "F32"
    elif tf_type == "float64":
        _dtype = "F64"
    else:
        raise ValueError(
            f"{tf_type} is not supported by aggregation server. \
            Federation will fail. Please use 'float32' or 'float64'."
        )
    return _dtype


def get_tf_model_dtype(model: tf.keras.Model) -> str:
    r"""."""
    if isinstance(model, tf.keras.Model):
        return translate_tf_model_dtype(model.dtype)
    else:
        raise TypeError(
            f"Unknown model type: {type(model)}. Consider to inherite from tf.keras.Model."
        )


def get_tf_model_shape(model: tf.keras.Model) -> str:
    r"""."""
    weights = model.get_weights()
    # print("weights: ", weights)

    print("get shape: ", get_shape(weights))

    for layer in model.layers:
        print(layer.output_shape)
