#include <NCollection_Vector.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Wire.hxx>
#include <TopExp_Explorer.hxx>

#include <type_traits>
#include <utility>

namespace brep_utils
{
	template <typename TopoShapeType>
	struct shape_traits { };

	template <>
	struct shape_traits<TopoDS_Vertex>
	{
		static constexpr TopAbs_ShapeEnum shape_type() { return TopAbs_VERTEX; }

		// std::function doesn't work here - will cause a crash
		static constexpr const TopoDS_Vertex& (*shape_fn)(const TopoDS_Shape&) = &TopoDS::Vertex;
	};

	template <>
	struct shape_traits<TopoDS_Edge>
	{
		static constexpr TopAbs_ShapeEnum shape_type() { return TopAbs_EDGE; }

		static constexpr const TopoDS_Edge& (*shape_fn)(const TopoDS_Shape&) = &TopoDS::Edge;
	};

	template <>
	struct shape_traits<TopoDS_Face>
	{
		static constexpr TopAbs_ShapeEnum shape_type() { return TopAbs_FACE; }

		static constexpr const TopoDS_Face& (*shape_fn)(const TopoDS_Shape&) = &TopoDS::Face;
	};

	template <>
	struct shape_traits<TopoDS_Wire>
	{
		static constexpr TopAbs_ShapeEnum shape_type() { return TopAbs_WIRE; }

		static constexpr const TopoDS_Wire& (*shape_fn)(const TopoDS_Shape&) = &TopoDS::Wire;
	};

	template <typename TopoShapeType>
	auto get_topo(const TopoDS_Shape & s) -> typename std::enable_if<std::is_base_of<TopoDS_Shape, TopoShapeType>::value, NCollection_Vector<TopoShapeType> >::type
	{
		NCollection_Vector<TopoShapeType> shapes;
		TopExp_Explorer exp_shape(s, shape_traits<TopoShapeType>::shape_type());
		for (; exp_shape.More() ; exp_shape.Next())
		{
			TopoDS_Shape cur_shape = exp_shape.Current();
			if (cur_shape.IsNull())
				continue;

			TopoShapeType shape_type = shape_traits<TopoShapeType>::shape_fn(cur_shape);
			shapes.Append(shape_type);
		}

		return shapes;
	}
};
