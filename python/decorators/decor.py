_GLOBAL_MODEL = []
# _ROUND_ID = 0

def train_decor(func):
    r"""."""

    def wrapper(model, *args, **kwargs):
        while wrapper.round_id < 10:
            model = _get_model()
            wrapper.round_id += 1

            print("Testing decorators. round: ", wrapper.round_id)

            func(model, *args, **kwargs)
            #
            # print("globals: ", model, " ", wrapper.round_id)
            # _update_model()
    wrapper.round_id = 0

    return wrapper


def _get_model():
    return [0.25, 7.4, 1.5, 2.8, 3.0]

# def _update_model(model):
#     pass
