from inkrecipes import quantities as q


def test_parse_quantity_forms():
    assert q.parse_quantity("300 g kimchi") == (300.0, "g kimchi")
    assert q.parse_quantity("1 1/2 cups rice")[0] == 1.5
    assert q.parse_quantity("½ tsp salt")[0] == 0.5
    assert q.parse_quantity("0.75 l water")[0] == 0.75
    assert q.parse_quantity("salt to taste") == (0.0, "salt to taste")


def test_parse_ingredient_units_metric_and_imperial():
    i = q.parse_ingredient("- 1 1/2 cups cooked rice")
    assert (i.quantity, i.unit, i.name) == (1.5, "cup", "cooked rice")
    i = q.parse_ingredient("- 2 tablespoons olive oil")
    assert (i.quantity, i.unit) == (2.0, "tbsp")
    i = q.parse_ingredient("- 250 g split red lentils, rinsed")
    assert (i.quantity, i.unit) == (250.0, "g")
    i = q.parse_ingredient("- 8 oz cheddar")
    assert (i.quantity, i.unit) == (8.0, "oz")
    i = q.parse_ingredient("- salt, to taste")
    assert (i.quantity, i.unit, i.name) == (0.0, "", "salt, to taste")
    i = q.parse_ingredient("- 2 spring onions, sliced")
    assert (i.quantity, i.unit, i.name) == (2.0, "", "spring onions, sliced")


def test_parse_duration():
    assert q.parse_duration_seconds("[5 min]") == 300
    assert q.parse_duration_seconds("1 hour 20 minutes") == 4800
    assert q.parse_duration_seconds("90 s") == 90
    assert q.parse_duration_seconds("no timer here") == 0
