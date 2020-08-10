namespace juce {
namespace {
    const Identifier ComponentScaleFactorProperty = "juce_ComponentScaleFactor-8959ABBD-AAC8-4307-A419-A343A8F55252";

    static float getComponentScaleFactorProperty(const Component& component) noexcept
    {
        if (const auto* scaleFactorProperty = component.getProperties().getVarPointer(ComponentScaleFactorProperty))
            return *scaleFactorProperty;

        return 1.f;
    }
}

void setComponentScaleFactor(Component& component, float scaleFactor) noexcept
{
    component.getProperties().set(ComponentScaleFactorProperty, scaleFactor);
    component.repaint();
    component.sendMovedResizedMessages(false, false);
}

float getComponentScaleFactor(const Component& component) noexcept
{
    float scaleFactor = getComponentScaleFactorProperty(component);

    if (component.isTransformed()) {
        const float scaleFactorFromTransform = sqrt(abs(component.getTransform().getDeterminant()));
        scaleFactor *= scaleFactorFromTransform;
    }

    return scaleFactor;
}

namespace {
    static Point<int> applyTransformAndComponentScaleFactor(const Component& comp, Point<int> p) noexcept
    {
        return (p.transformedBy(comp.getTransform()) * getComponentScaleFactorProperty(comp)).roundToInt();
    }

    static Point<int> applyInvertedTransformAndComponentScaleFactor(const Component& comp, Point<int> p) noexcept
    {
        return (p.transformedBy(comp.getTransform().inverted()) / getComponentScaleFactorProperty(comp)).roundToInt();
    }

    static Point<float> applyTransformAndComponentScaleFactor(const Component& comp, Point<float> p) noexcept
    {
        return (p.transformedBy(comp.getTransform()) * getComponentScaleFactorProperty(comp));
    }

    static Point<float> applyInvertedTransformAndComponentScaleFactor(const Component& comp, Point<float> p) noexcept
    {
        return (p.transformedBy(comp.getTransform().inverted()) / getComponentScaleFactorProperty(comp));
    }

    static Rectangle<int> applyTransformAndComponentScaleFactor(const Component& comp, Rectangle<int> r) noexcept
    {
        return (r.transformedBy(comp.getTransform()) * getComponentScaleFactorProperty(comp)).toNearestIntEdges();
    }

    static Rectangle<int> applyInvertedTransformAndComponentScaleFactor(const Component& comp, Rectangle<int> r) noexcept
    {
        return (r.transformedBy(comp.getTransform().inverted()) / getComponentScaleFactorProperty(comp)).toNearestIntEdges();
    }

    static bool hasTransformOrComponentScaleFactor(const Component& comp) noexcept
    {
        return (comp.isTransformed() || getComponentScaleFactorProperty(comp) != 1.f);
    }

    static AffineTransform getTransformWithComponentScaleFactor(const Component& comp) noexcept
    {
        return comp.getTransform().scaled(getComponentScaleFactorProperty(comp));
    }

    static AffineTransform applyTransformAndComponentScaleFactor(const AffineTransform& sourceTransform, const Component& targetComponent) noexcept
    {
        return sourceTransform.followedBy(getTransformWithComponentScaleFactor(targetComponent));
    }
}
}
