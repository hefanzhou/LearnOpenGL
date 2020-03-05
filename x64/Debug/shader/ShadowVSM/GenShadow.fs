#version 330 core
in vec4 fragPos;
out vec4 FragColor;  
void main()
{

	float depth = fragPos.z / fragPos.w;
    depth = depth * 0.5 + 0.5;// TO NDC [0, 1]

    float moment1 = depth; // 一阶矩
    float moment2 = depth * depth; // 二阶矩

    float dx = dFdx(depth);
    float dy = dFdy(depth);
    moment2 += 0.25 * (dx * dx + dy * dy); // 解决acne问题
    FragColor = vec4(moment1, moment2, 0, 1);
}