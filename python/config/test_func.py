import copy
from dataclasses import dataclass, field
from typing import Any, Dict, List

import yaml


@dataclass
class Config(object):
    r"""Base configuration class."""

    NAME: str = None
    GPU_COUNT: int = 1
    IMAGES_PER_GPU: int = 2
    STEPS_PER_EPOCH: int = 1000

    def merge_cfg_from_yaml_str(self, str_obj):
        """Load a config from a YAML string encoding."""
        with open(str_obj, "r") as yf:
            try:
                cfg_as_dict = yaml.safe_load(yf)
                self._merge_a_into_self(cfg_as_dict, self, [])
            except yaml.YAMLError as exc:
                print(exc)

    def __iter__(self):
        """Create an iterator."""
        yield self.__dataclass_fields__

    def __setitem__(self, key, value):
        """."""
        setattr(self, key, value)

    # def __setitem__(self, key, value):
    #     """."""
    #     value = self._check_and_coerce_conf_value_type(value, self[key])
    #     setattr(self, key, value)
    #
    def __getitem__(self, key):
        """."""
        try:
            return getattr(self, key)
        except:
            print(f"AttributeError: {key}")

    def _merge_a_into_self(self, external_d, cfg, key_list: List[str]):
        """Merge a config dictionary a into self, clobbering the
        options in b whenever they are also specified in a.

        :param external_d: External dictionary extracted from .yaml.
        :param conf: Config object that should be merged into.
        """
        for k, v_ in external_d.items():
            full_key = ".".join(key_list + [k])
            v = copy.deepcopy(v_)

            if hasattr(cfg, k):
                v = self._check_and_coerce_conf_value_type(v, cfg[k])
                cfg[k] = v

    @staticmethod
    def _check_and_coerce_conf_value_type(replacement, original, casts: List[List[Any]] = [[(tuple, list), (list, tuple)]], valid_types: Dict = {tuple, list, dict, str, int, float, bool, type(None)}):
        """Checks that `replacement`, which is intended to replace `original` is of
        the right type. The type is correct if it matches exactly or is one of a few
        cases in which the type can be easily coerced.

        :param replacement: Intended replacement parameter.
        :param original: Value to be replaced.
        :param casts: List

        """
        original_type = type(original)
        replacement_type = type(replacement)

        if replacement_type == original_type:
            return replacement

        if (replacement_type == type(None) and original_type in valid_types) or (original_type == type(None) and replacement_type in valid_types):
            return replacement

        # Cast replacement from from_type to to_type if the replacement and original
        # types match from_type and to_type
        def conditional_cast(from_type, to_type):
            if replacement_type == from_type and original_type == to_type:
                return True, to_type(replacement)
            else:
                return False, None

        for cast_pair in casts:
            for (from_type, to_type) in casts:
                converted, converted_value = conditional_cast(from_type, to_type)
                if converted:
                    return converted_value

        raise ValueError(
            f"Type mismatch ({original_type} vs. {replacement_type}) with values ({original} vs. {replacement}) for Conf."
        )


obj = Config()
obj.merge_cfg_from_yaml_str("config.yaml")
print(obj)
obj["TEST"] = 10
obj["NAME"] = "hello."
print(obj)
obj.NAME = 10
print(obj)
