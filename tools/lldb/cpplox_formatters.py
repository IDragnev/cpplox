import lldb

MAX_CHILDREN = 10000


class VectorSynthProvider:
    def __init__(self, valobj, internal_dict):
        self.valobj = valobj
        self.update()

    def update(self):
        self.count = self.valobj.GetChildMemberWithName("count").GetValueAsUnsigned(0)
        self.size = self.valobj.GetChildMemberWithName("size").GetValueAsUnsigned(0)
        self.items = self.valobj.GetChildMemberWithName("items")
        self.elem_type = self.items.GetType().GetPointeeType()
        self.elem_size = self.elem_type.GetByteSize()
        return False

    def num_children(self):
        # Uninitialized or moved-from vectors can hold garbage counts.
        if self.count > self.size or self.items.GetValueAsUnsigned(0) == 0:
            return 0
        return min(self.count, MAX_CHILDREN)

    def has_children(self):
        return True

    def get_child_index(self, name):
        try:
            return int(name.strip("[]"))
        except ValueError:
            return -1

    def get_child_at_index(self, index):
        if index < 0 or index >= self.num_children():
            return None
        address = self.items.GetValueAsUnsigned(0) + index * self.elem_size
        return self.valobj.CreateValueFromAddress(f"[{index}]", address, self.elem_type)


def vector_summary(valobj, internal_dict):
    raw = valobj.GetNonSyntheticValue()
    count = raw.GetChildMemberWithName("count").GetValueAsUnsigned(0)
    size = raw.GetChildMemberWithName("size").GetValueAsUnsigned(0)
    return f"count={count} size={size}"


def __lldb_init_module(debugger, internal_dict):
    pattern = '"^cpplox::Vector<.+>$"'
    debugger.HandleCommand(f"type synthetic add -x {pattern} -l {__name__}.VectorSynthProvider")
    debugger.HandleCommand(f"type summary add --expand -x {pattern} -F {__name__}.vector_summary")
