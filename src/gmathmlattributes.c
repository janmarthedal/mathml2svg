/* gmathmlattributes.c
 *
 * Copyright (C) 2007  Emmanuel Pacaud
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 *
 * Author:
 * 	Emmanuel Pacaud <emmanuel@gnome.org>
 */

#include <gmathmlattributes.h>
#include <string.h>
#include <stdlib.h>
#include <glib/gmem.h>
#include <glib/ghash.h>

typedef struct {
	const char * 	(*to_string)	(GMathmlAttribute *attr);
	void 		(*from_string)	(GMathmlAttribute *attr, const char *string);
	void 		(*finalize) 	(GMathmlAttribute *attr);
} GMathmlAttributeClass;

/* Boolean attribute */

static const char *
gmathml_boolean_attribute_to_string (GMathmlAttribute *attr)
{
	GMathmlBooleanAttribute *bool_attr = (GMathmlBooleanAttribute *) attr;

	return (bool_attr->value ? "true" : "false");
}

static void
gmathml_boolean_attribute_from_string (GMathmlAttribute *attr, const char *string)
{
	GMathmlBooleanAttribute *bool_attr = (GMathmlBooleanAttribute *) attr;

	if (string != NULL &&
	    strcmp ("true", string) == 0)
		bool_attr->value = TRUE;
	else
		bool_attr->value = FALSE;
}

void
gmathml_boolean_attribute_set_default (GMathmlBooleanAttribute *self,
				       gboolean value)
{
	g_return_if_fail (self != NULL);

	if (!self->is_defined)
		self->value = value;
}

/* Integer attribute */

static const char *
gmathml_int_attribute_to_string (GMathmlAttribute *attr)
{
	GMathmlIntAttribute *int_attr = (GMathmlIntAttribute *) attr;

	if (int_attr->string == NULL)
		int_attr->string = g_strdup_printf ("%i", int_attr->value);

	return int_attr->string;
}

static void
gmathml_int_attribute_from_string (GMathmlAttribute *attr, const char *string)
{
	GMathmlIntAttribute *int_attr = (GMathmlIntAttribute *) attr;

	g_free (int_attr->string);
	int_attr->string = NULL;

	if (string != NULL)
		int_attr->value = atoi (string);
	else
		int_attr = 0;
}

static void
gmathml_int_attribute_finalize (GMathmlAttribute *attr)
{
	GMathmlIntAttribute *int_attr = (GMathmlIntAttribute *) attr;

	g_free (int_attr->string);
	int_attr->string = NULL;
}

void
gmathml_int_attribute_set_default (GMathmlIntAttribute *self,
				   int value)
{
	g_return_if_fail (self != NULL);

	if (!self->is_defined) {
		g_free (self->string);
		self->string = NULL;
		self->value = value;
	}
}

/* Increment attribute ([+|-] integer) */

static const char *
gmathml_increment_attribute_to_string (GMathmlAttribute *attr)
{
	GMathmlIncrementAttribute *increment_attr = (GMathmlIncrementAttribute *) attr;

	if (increment_attr->string == NULL) {
		switch (increment_attr->type) {
			case GMATHML_LEVEL_TYPE_UP:
				increment_attr->string = g_strdup_printf ("+%ui", increment_attr->value);
				break;
			case GMATHML_LEVEL_TYPE_DOWN:
				increment_attr->string = g_strdup_printf ("-%ui", increment_attr->value);
				break;
			case GMATHML_LEVEL_TYPE_ABSOLUTE:
			default:
				increment_attr->string = g_strdup_printf ("%ui", increment_attr->value);
				break;
		}
	}

	return increment_attr->string;
}

static void
gmathml_increment_attribute_from_string (GMathmlAttribute *attr, const char *string)
{
	GMathmlIncrementAttribute *increment_attr = (GMathmlIncrementAttribute *) attr;

	g_free (increment_attr->string);
	increment_attr->string = NULL;

	if (string != NULL) {
		increment_attr->value = atoi (string);
		if (*string == '+')
			increment_attr->type = GMATHML_LEVEL_TYPE_UP;
		else if (*string == '-')
			increment_attr->type = GMATHML_LEVEL_TYPE_DOWN;
		else
			increment_attr->type = GMATHML_LEVEL_TYPE_ABSOLUTE;
	} else {
		increment_attr->value = 0;
		increment_attr->type = GMATHML_LEVEL_TYPE_UP;
	}
}

static void
gmathml_increment_attribute_finalize (GMathmlAttribute *attr)
{
	GMathmlIncrementAttribute *increment_attr = (GMathmlIncrementAttribute *) attr;

	g_free (increment_attr->string);
	increment_attr->string = NULL;
}

void
gmathml_increment_attribute_set_default (GMathmlIncrementAttribute *self,
					 unsigned int value,
					 GMathmlLevelType type)
{
	g_return_if_fail (self != NULL);

	if (!self->is_defined) {
		g_free (self->string);
		self->string = NULL;
		self->value = value;
		self->type = type;
	}
}

/* Double attribute */

static const char *
gmathml_double_attribute_to_string (GMathmlAttribute *attr)
{
	GMathmlDoubleAttribute *double_attr = (GMathmlDoubleAttribute *) attr;

	if (double_attr->string == NULL)
		double_attr->string = g_strdup_printf ("%g", double_attr->value);

	return double_attr->string;
}

static void
gmathml_double_attribute_from_string (GMathmlAttribute *attr, const char *string)
{
	GMathmlDoubleAttribute *double_attr = (GMathmlDoubleAttribute *) attr;

	g_free (double_attr->string);
	double_attr->string = NULL;

	if (string != NULL)
		double_attr->value = g_strtod (string, NULL);
	else
		double_attr = 0;
}

static void
gmathml_double_attribute_finalize (GMathmlAttribute *attr)
{
	GMathmlDoubleAttribute *double_attr = (GMathmlDoubleAttribute *) attr;

	g_free (double_attr->string);
	double_attr->string = NULL;
}

void
gmathml_double_attribute_set_default (GMathmlDoubleAttribute *self,
				      double value)
{
	g_return_if_fail (self != NULL);

	if (!self->is_defined) {
		g_free (self->string);
		self->string = NULL;
		self->value = value;
	}
}

/* Length attribute (number [unit])*/

static const char *
gmathml_length_attribute_to_string (GMathmlAttribute *attr)
{
	GMathmlLengthAttribute *length_attr = (GMathmlLengthAttribute *) attr;

	if (length_attr->string == NULL)
		length_attr->string = g_strdup_printf ("%g%s",
						       length_attr->value,
						       gmathml_unit_to_string (length_attr->unit));

	return length_attr->string;
}

static void
gmathml_length_attribute_from_string (GMathmlAttribute *attr, const char *string)
{
	GMathmlLengthAttribute *length_attr = (GMathmlLengthAttribute *) attr;

	g_free (length_attr->string);
	length_attr->string = NULL;

	if (string != NULL) {
		char *unit_str;

		length_attr->value = g_strtod (string, &unit_str);
		length_attr->unit = gmathml_unit_from_string (unit_str);
		if (length_attr->unit == GMATHML_UNIT_NONE)
			length_attr->value *= 100.0;
	} else {
		length_attr->value = 0;
		length_attr->unit = GMATHML_UNIT_PX;
	}
}

static void
gmathml_length_attribute_finalize (GMathmlAttribute *attr)
{
	GMathmlLengthAttribute *length_attr = (GMathmlLengthAttribute *) attr;

	g_free (length_attr->string);
	length_attr->string = NULL;
}

void
gmathml_length_attribute_set_default (GMathmlLengthAttribute *self,
				      double value,
				      GMathmlUnit unit)
{
	g_return_if_fail (self != NULL);

	if (!self->is_defined) {
		g_free (self->string);
		self->string = NULL;
		self->value = value;
		self->unit = unit;
	}
}

/* String attribute */

static const char *
gmathml_string_attribute_to_string (GMathmlAttribute *attr)
{
	GMathmlStringAttribute *str_attr = (GMathmlStringAttribute *) attr;

	return str_attr->string;
}

static void
gmathml_string_attribute_from_string (GMathmlAttribute *attr, const char *string)
{
	GMathmlStringAttribute *str_attr = (GMathmlStringAttribute *) attr;

	g_free (str_attr->string);

	if (string != NULL)
		str_attr->string = g_strdup (string);
	else
		str_attr->string = NULL;
}

static void
gmathml_string_attribute_finalize (GMathmlAttribute *attr)
{
	GMathmlStringAttribute *str_attr = (GMathmlStringAttribute *) attr;

	g_free (str_attr->string);
	str_attr->string = NULL;
}

void
gmathml_string_attribute_set_default (GMathmlStringAttribute *self,
				      const char *string)
{
	g_return_if_fail (self != NULL);

	if (!self->is_defined) {
		g_free (self->string);
		self->string = string != NULL ? g_strdup (string) : NULL;
	}
}

/* Color attribute */

static const char *
gmathml_color_attribute_to_string (GMathmlAttribute *attr)
{
	GMathmlColorAttribute *color_attr = (GMathmlColorAttribute *) attr;

	if (color_attr->string != NULL) {
		color_attr->string = pango_color_to_string (&color_attr->color);
	}

	return color_attr->string;
}

static void
gmathml_color_attribute_from_string (GMathmlAttribute *attr, const char *string)
{
	GMathmlColorAttribute *color_attr = (GMathmlColorAttribute *) attr;

	g_free (color_attr->string);

	if (string != NULL) {
		color_attr->string = g_strdup (string);
		pango_color_parse (&color_attr->color, color_attr->string);
	} else
		color_attr->string = NULL;
}

static void
gmathml_color_attribute_finalize (GMathmlAttribute *attr)
{
	GMathmlColorAttribute *color_attr = (GMathmlColorAttribute *) attr;

	g_free (color_attr->string);
	color_attr->string = NULL;
}

void
gmathml_color_attribute_set_default (GMathmlColorAttribute *self,
				     const PangoColor *color)
{
	g_return_if_fail (self != NULL);

	if (!self->is_defined) {
		g_free (self->string);
		self->string = NULL;
		self->color = *color;
	}
}

static const GMathmlAttributeClass gmathml_attribute_classes[] = {
	{
		gmathml_boolean_attribute_to_string,
		gmathml_boolean_attribute_from_string,
		NULL
	},
	{
		gmathml_int_attribute_to_string,
		gmathml_int_attribute_from_string,
		gmathml_int_attribute_finalize
	},
	{
		gmathml_increment_attribute_to_string,
		gmathml_increment_attribute_from_string,
		gmathml_increment_attribute_finalize
	},
	{
		gmathml_double_attribute_to_string,
		gmathml_double_attribute_from_string,
		gmathml_double_attribute_finalize
	},
	{
		gmathml_length_attribute_to_string,
		gmathml_length_attribute_from_string,
		gmathml_length_attribute_finalize
	},
	{
		gmathml_string_attribute_to_string,
		gmathml_string_attribute_from_string,
		gmathml_string_attribute_finalize
	},
	{
		gmathml_color_attribute_to_string,
		gmathml_color_attribute_from_string,
		gmathml_color_attribute_finalize
	}
};

typedef struct {
	const GMathmlAttributeClass *attr_class;
	ptrdiff_t attr_offset;
} GMathmlAttributeInfos;

typedef struct GHashTable _GMathmlAttributes;

GMathmlAttributes *
gmathml_attributes_new (void)
{
	GHashTable *hash;

	hash = g_hash_table_new_full (g_str_hash, g_str_equal, NULL, NULL);

	return (GMathmlAttributes *) hash;
}

void
gmathml_attributes_add_attribute (GMathmlAttributes *attributes,
				  const char *attr_name,
				  GMathmlAttributeType attr_type,
				  ptrdiff_t attr_offset)
{
	GHashTable *hash = (GHashTable *) attributes;
	GMathmlAttributeInfos *attr_infos;

	g_return_if_fail (attributes != NULL);
	g_return_if_fail (attr_name != NULL);
	g_return_if_fail (attr_type >= 0 && attr_type < G_N_ELEMENTS (gmathml_attribute_classes));
	g_return_if_fail (attr_offset >= 0);

	if (g_hash_table_lookup (hash, attr_name) != NULL) {
		g_warning ("[GMathmlAttributes::add_attribute] %s already defined", attr_name);
		return;
	}

	attr_infos = g_new (GMathmlAttributeInfos, 1);
	attr_infos->attr_class = &gmathml_attribute_classes [attr_type];
	attr_infos->attr_offset = attr_offset;

	g_hash_table_insert (hash, (char *) attr_name, attr_infos);
}

gboolean
gmathml_attributes_set_attribute (GMathmlAttributes *attributes,
				  void *instance,
				  const char *attr_name,
				  const char *attr_value)
{
	GHashTable *hash = (GHashTable *) attributes;
	GMathmlAttributeInfos *attr_infos;

	g_return_val_if_fail (attributes != NULL, FALSE);

	attr_infos = g_hash_table_lookup (hash, attr_name);
	if (attr_infos == NULL)
		return FALSE;

	attr_infos->attr_class->from_string (instance + attr_infos->attr_offset, attr_value);
	((GMathmlAttribute *)((void *)(instance + attr_infos->attr_offset)))->is_defined = TRUE;

	return TRUE;
}

char const *
gmathml_attributes_get_attribute (GMathmlAttributes *attributes,
				  void *instance,
				  const char *attr_name)
{
	GHashTable *hash = (GHashTable *) attributes;
	GMathmlAttributeInfos *attr_infos;

	g_return_val_if_fail (attributes != NULL, FALSE);

	attr_infos = g_hash_table_lookup (hash, attr_name);
	if (attr_infos == NULL)
		return NULL;

	return attr_infos->attr_class->to_string (instance + attr_infos->attr_offset);
}

gboolean
gmathml_attributes_is_attribute_defined (GMathmlAttributes *attributes,
					 void *instance,
					 const char *attr_name)
{
	GHashTable *hash = (GHashTable *) attributes;
	GMathmlAttributeInfos *attr_infos;

	g_return_val_if_fail (attributes != NULL, FALSE);

	attr_infos = g_hash_table_lookup (hash, attr_name);
	if (attr_infos == NULL)
		return FALSE;

	return ((GMathmlAttribute *)((void *)(instance + attr_infos->attr_offset)))->is_defined;
}

static void
gmathml_attribute_finalize_cb (gpointer key,
			       gpointer value,
			       gpointer instance)
{
	GMathmlAttributeInfos *attr_infos = value;

	if (attr_infos->attr_class->finalize)
		attr_infos->attr_class->finalize (instance + attr_infos->attr_offset);
}

void
gmathml_attributes_finalize_attributes (GMathmlAttributes *attributes, void *instance)
{
	GHashTable *hash = (GHashTable *) attributes;

	g_return_if_fail (attributes != NULL);

	g_hash_table_foreach (hash, gmathml_attribute_finalize_cb, instance);
}
