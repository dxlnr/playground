import os

os.environ["TF_CPP_MIN_LOG_LEVEL"] = "3"

import tensorflow as tf
from tf_utils import get_tf_model_dtype, get_tf_model_shape

# Custom keras model using Sequential API
seq_model = tf.keras.models.Sequential(
    [
        tf.keras.layers.Flatten(input_shape=(32, 32, 3)),
        tf.keras.layers.Dense(3000, activation="relu"),
        tf.keras.layers.Dense(1000, activation="relu"),
        tf.keras.layers.Dense(10),
    ]
)
# print(get_tf_model_shape(seq_model))
# print('\n')

# weights_seq = seq_model.get_weights()
# print('\n')
# print(len(weights_seq))
# print("\n")
# seq_model.summary()
# dtype = get_tf_model_dtype(seq_model)
# print(dtype)

#
# --------------------------------------------------------------------------- #
#
#
# Keras model using Functional API
func_model = tf.keras.applications.MobileNetV2((32, 32, 3), classes=10, weights=None)
# func_model.compile("adam", "sparse_categorical_crossentropy", metrics=["accuracy"])
# print(get_tf_model_shape(func_model))
# print('\n')

# (x_train, y_train), (x_test, y_test) = tf.keras.datasets.cifar10.load_data()
# func_model.fit(x_train, y_train, epochs=1, batch_size=32)

# weights = func_model.get_weights()
# print("weights: ")
# print(type(weights))
# print(type(weights[0]))
# print(len(weights))
# print("\n")
#
# print(type(func_model))
# print(isinstance(func_model, tf.keras.Model))
# print(get_tf_model_dtype(func_model))
# # func_model.summary()


#
# --------------------------------------------------------------------------- #
#
# Keras model using Model Sub-Classing API
class SubclassedModel(tf.keras.Model):
    def __init__(self):
        super().__init__()
        self.conv1 = tf.keras.layers.Conv2D(32, 3, strides=2, activation="relu")
        self.max1 = tf.keras.layers.MaxPooling2D(3)
        self.bn1 = tf.keras.layers.BatchNormalization()

        self.dense = tf.keras.layers.Dense(5)

    def call(self, input_tensor, training=False):
        x = self.conv1(input_tensor)
        x = self.max1(x)
        x = self.bn1(x)

        return self.dense(x)


sub_model = SubclassedModel()
sub_model.compile(
    loss=tf.keras.losses.CategoricalCrossentropy(),
    metrics=tf.keras.metrics.CategoricalAccuracy(),
    optimizer=tf.keras.optimizers.Adam(),
)
print(get_tf_model_shape(sub_model))
